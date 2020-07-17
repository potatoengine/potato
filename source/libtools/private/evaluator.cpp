// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "evaluator.h"

#include "potato/spud/ascii.h"
#include "potato/spud/hash.h"

up::tools::Evaluator::Evaluator() {
    // Nul so that EvaluatorId{} always results in false
    //
    (void)_add({.hash = 0, .op = Op::Literal});
}

up::tools::Evaluator::~Evaluator() = default;

void up::tools::Evaluator::set(string_view name, string_view value) {
    auto const nameHash = hash_value(name);
    auto const valueHash = hash_value(value);

    _set(nameHash, valueHash);
}

void up::tools::Evaluator::set(string_view name, bool value) {
    auto const nameHash = hash_value(name);
    auto constexpr yesHash = 1;
    auto constexpr noHash = 0;

    _set(nameHash, value ? yesHash : noHash);
}

void up::tools::Evaluator::_set(NameHash name, Value value) {
    for (auto& var : _variables) {
        if (var.name == name) {
            if (var.value != value) {
                var.value = value;
                ++_revision;
            }
            return;
        }
    }

    _variables.push_back(Variable{.name = name, .value = value});
}

auto up::tools::Evaluator::_get(NameHash name, Value defaultValue) const noexcept -> Value {
    for (auto& var : _variables) {
        if (var.name == name) {
            return var.value;
        }
    }
    return defaultValue;
}

void up::tools::Evaluator::clear(string_view name) {
    auto const hash = hash_value(name);

    for (auto& var : _variables) {
        if (var.name == hash) {
            std::swap(var, _variables.back());
            _variables.pop_back();
            ++_revision;
            return;
        }
    }
}

auto up::tools::Evaluator::_add(Expr expr) -> Index {
    expr.version = _revision - 1;
    for (auto& e : _exprs) {
        if (e.hash == expr.hash) {
            e = expr;
            return static_cast<Index>(&e - _exprs.data());
        }
    }
    auto const index = static_cast<Index>(_exprs.size());
    _exprs.push_back(expr);
    return index;
}

auto up::tools::Evaluator::_add(Op op, Index arg1, Index arg0) -> Index {
    auto const hash = hash_combine(hash_value(op), hash_combine(hash_value(arg0), hash_value(arg1)));
    return _add({.hash = hash, .arg0 = arg0, .arg1 = arg1, .op = op});
}

auto up::tools::Evaluator::_add(Op op, Index arg) -> Index {
    auto const hash = hash_combine(hash_value(op), hash_value(arg));
    return _add({.hash = hash, .arg0 = arg, .op = op});
}

auto up::tools::Evaluator::_add(Op op, NameHash var) -> Index {
    return _add({.hash = var, .op = op});
}

auto up::tools::Evaluator::compile(string_view expr) -> EvaluatorId {
    enum class Token { Unknown, Identifier, Number, String, LParen, RParen, Equals, NotEquals, Not, And, Or, End };

    // Shunting Yard state; values are expression indices
    //
    vector<Index> values;
    vector<Token> ops;

    // Operator precedence tables
    //
    constexpr int precedence_table[] = {
        0, // Unknown
        0, // Identifier
        0, // Number
        0, // String
        -1, // LParen
        9, // RParen
        2, // Equals
        2, // NotEquals
        1, // Not
        4, // And
        3, // Or
        0, // End
    };

    auto const precedence = [precedence_table](Token op) noexcept -> int {
        return precedence_table[static_cast<int>(op)];
    };

    // Pop a value from the value stack, protecting against empty values (bad input)
    //
    auto pop = [&]() noexcept -> Index {
        if (values.empty()) {
            return {};
        }
        auto const value = values.back();
        values.pop_back();
        return value;
    };

    // Given an operator from the top of the stack, pop the corresponding number of values
    // and push a new values onto the stack formed by the operator. e.g. the `And` operator
    // will pop two values and then push a new Conjunction with those operands.
    //
    auto apply = [&, this]() {
        auto const op = ops.back();
        ops.pop_back();
        switch (op) {
            case Token::Equals:
                values.push_back(_add(Op::Equality, pop(), pop()));
                break;
            case Token::NotEquals:
                values.push_back(_add(Op::Inequality, pop(), pop()));
                break;
            case Token::Not:
                values.push_back(_add(Op::Complement, pop()));
                break;
            case Token::And:
                values.push_back(_add(Op::Conjunction, pop(), pop()));
                break;
            case Token::Or:
                values.push_back(_add(Op::Disjunction, pop(), pop()));
                break;
            default:
                break;
        }
    };

    // Push a new operator token onto the stack, after first collapsing any lower-priority
    // operators.
    //
    auto push = [&](Token op) {
        int const prec = precedence(op);
        int backPrec = -1;
        while (!ops.empty() && (backPrec = precedence(ops.back())) <= prec && backPrec >= 0) {
            apply();
        }
        ops.push_back(op);
    };

    // Parse a string
    //
    auto parseString = [](string_view& in) noexcept -> string_view {
        char const quote = in.front();
        in.pop_front();

        string_view::size_type length = 0;
        while (length < in.size() && in[length] != quote) {
            ++length;
        }
        if (length == in.size()) {
            return {};
        }

        auto const data = in.substr(0, length);
        in = in.substr(length + 1);

        return data;
    };

    // Consume an identifier (assumes first character is a legal identifier start char
    //
    auto parseIdentifier = [](string_view& in) noexcept -> string_view {
        string_view::size_type length = 1;
        while (length < in.size() && (ascii::is_alnum(in[length]) || in[length] == '.')) {
            ++length;
        }
        auto const data = in.substr(0, length);
        in = in.substr(length);
        return data;
    };

    // Consume a token from the input source and return its type and payload (if appropriate)
    //
    struct Parsed {
        Token token;
        string_view payload;
    };
    auto consume = [parseString, parseIdentifier](string_view& in) noexcept -> Parsed {
        while (!in.empty() && in.front() == ' ') {
            in.pop_front();
        }

        if (in.empty()) {
            return {Token::End};
        }

        char const c = in.front();

        if (c == '(') {
            in.pop_front();
            return {Token::LParen};
        }

        if (c == ')') {
            in.pop_front();
            return {Token::RParen};
        }

        if (c == '\'') {
            return {.token = Token::String, .payload = parseString(in)};
        }

        if (c == '!') {
            if (in.starts_with("!=")) {
                in = in.substr(2);
                return {Token::NotEquals};
            }
            else {
                in.pop_front();
                return {Token::Not};
            }
        }

        if (in.starts_with("&&")) {
            in = in.substr(2);
            return {Token::And};
        }

        if (in.starts_with("||")) {
            in = in.substr(2);
            return {Token::Or};
        }

        if (in.starts_with("==")) {
            in = in.substr(2);
            return {Token::Equals};
        }

        if (ascii::is_alpha(c)) {
            return {.token = Token::Identifier, .payload = parseIdentifier(in)};
        }

        return {Token::Unknown};
    };

    // Shunting Yard over the input tokens
    //
    while (!expr.empty()) {
        auto const [token, data] = consume(expr);
        switch (token) {
            case Token::Identifier:
                values.push_back(_add(Op::Variable, hash_value(data)));
                break;
            case Token::String:
                values.push_back(_add(Op::Literal, hash_value(data)));
                break;
            case Token::Not:
            case Token::Equals:
            case Token::NotEquals:
            case Token::And:
            case Token::Or:
            case Token::LParen:
                push(token);
                break;
            case Token::RParen:
                while (!ops.empty() && ops.back() != Token::LParen) {
                    apply();
                }
                if (ops.empty()) {
                    return {};
                }
                ops.pop_back();
                break;
            case Token::End:
                break;
            default:
                return {};
        }
    }

    // Apply any remaining operators
    //
    while (!ops.empty()) {
        apply();
    }

    // If we don't have exactly 1 value left, our input was mal-formed
    //
    if (values.size() != 1) {
        return {};
    }

    return EvaluatorId{values.front()};
}

auto up::tools::Evaluator::evaluate(EvaluatorId id) noexcept -> bool {
    auto const index = to_underlying(id);
    if (index < 1 || index >= _exprs.size()) {
        return false;
    }

    return _evaluate(index) != 0;
}

auto up::tools::Evaluator::_evaluate(Index index) noexcept -> Value {
    auto& expr = _exprs[index];

    // There's no point to going through the memoization for literals
    //
    if (expr.op == Op::Literal) {
        return expr.hash;
    }

    // If the expression is memo'ized, we need do no further calculation
    //
    if (expr.version == _revision) {
        return expr.memo;
    }

    // Evaluate the expression
    //
    Value result = 0;
    switch (expr.op) {
        case Op::Variable:
            result = _get(expr.hash);
            break;
        case Op::Complement:
            result = _evaluate(expr.arg0) ? 0 : 1;
            break;
        case Op::Conjunction:
            result = _evaluate(expr.arg0) != 0 && _evaluate(expr.arg1) != 0;
            break;
        case Op::Disjunction:
            result = _evaluate(expr.arg0) != 0 || _evaluate(expr.arg1) != 0;
            break;
        case Op::Equality:
            result = (_evaluate(expr.arg0) == _evaluate(expr.arg1)) ? 1 : 0;
            break;
        case Op::Inequality:
            result = (_evaluate(expr.arg0) != _evaluate(expr.arg1)) ? 1 : 0;
            break;
        default:
            break;
    }

    // Memoize the result to avoid repeated calculation
    //
    expr.memo = result;
    expr.version = _revision;

    return result;
}
