// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

void up::shell::CommandContext::set(string_view name, string_view value) {
    auto const hash = hash_value(name);

    for (auto& val : _values) {
        if (val.var == hash) {
            if (string_view{val.value} != value) {
                val.value = value;
                _memos.clear();
            }
            return;
        }
    }

    _values.push_back(Value{.var = hash, .value = value});
}

void up::shell::CommandContext::set(string_view name, bool value) {
    set(name, value ? "YES"_sv : ""_sv);
}

auto up::shell::CommandContext::get(string_view name, string_view defaultValue) const noexcept -> string_view {
    return _get(hash_value(name), defaultValue);
}

auto up::shell::CommandContext::_get(uint64 var, string_view defaultValue) const noexcept -> string_view {
    for (auto& val : _values) {
        if (val.var == var) {
            return val.value;
        }
    }
    return defaultValue;
}

void up::shell::CommandContext::clear(string_view name) {
    auto const hash = hash_value(name);

    for (auto it = _values.begin(); it != _values.end(); ++it) {
        if (it->var == hash) {
            _values.erase(it);
            _memos.clear();
            return;
        }
    }
}

auto up::shell::CommandContext::variable(string_view name) -> CommandContextId {
    auto const hash = hash_value(name);
    return _add({.op = Op::Variable, .id = hash, .arg0 = hash});
}

auto up::shell::CommandContext::negate(CommandContextId exprId) -> CommandContextId {
    auto const hash = hash_combine(hash_value(Op::Negate), uint64{exprId});
    return _add({.op = Op::Negate, .id = hash, .arg0 = uint64{exprId}});
}

auto up::shell::CommandContext::conjuction(CommandContextId leftId, CommandContextId rightId) -> CommandContextId {
    auto const hash = hash_combine(hash_value(Op::Conjunction), hash_combine(uint64{leftId}, uint64{rightId}));
    return _add({.op = Op::Conjunction, .id = hash, .arg0 = uint64{leftId}, .arg1 = uint64{rightId}});
}

auto up::shell::CommandContext::disjunction(CommandContextId leftId, CommandContextId rightId) -> CommandContextId {
    auto const hash = hash_combine(hash_value(Op::Disjunction), hash_combine(uint64{leftId}, uint64{rightId}));
    return _add({.op = Op::Disjunction, .id = hash, .arg0 = uint64{leftId}, .arg1 = uint64{rightId}});
}

auto up::shell::CommandContext::_add(Expr expr) -> CommandContextId {
    for (auto const& e : _exprs) {
        if (e.id == expr.id) {
            return CommandContextId{expr.id};
        }
    }
    _exprs.push_back(expr);
    return CommandContextId{expr.id};
}

auto up::shell::CommandContext::compile(string_view when) -> CommandContextId {
    return variable(when);
}

auto up::shell::CommandContext::_evaluate(uint64 id) noexcept -> bool {
    for (auto const& memo : _memos) {
        if (memo.id == id) {
            return memo.result;
        }
    }

    for (auto const& expr : _exprs) {
        if (expr.id == id) {
            bool result = false;
            switch (expr.op) {
                case Op::Variable:
                    result = !_get(expr.arg0).empty();
                    break;
                case Op::Negate:
                    result = !_evaluate(expr.arg0);
                    break;
                case Op::Conjunction:
                    result = _evaluate(expr.arg0) && _evaluate(expr.arg1);
                    break;
                case Op::Disjunction:
                    result = _evaluate(expr.arg0) || _evaluate(expr.arg1);
                    break;
                default:
                    break;
            }
            _memos.push_back({.id = id, .result = result});
            return result;
        }
    }

    return false;
}

bool up::shell::CommandRegistry::_evaluate(CommandContextId id) noexcept {
    return id == CommandContextId{} ? true : _context.evaluate(id);
}

void up::shell::CommandRegistry::registerCommand(Command command) {
    if (!command.enablement.empty()) {
        command.enablementId = _context.compile(command.enablement);
    }
    if (!command.when.empty()) {
        command.whenId = _context.compile(command.when);
    }
    _commands.push_back(std::move(command));
}

void up::shell::CommandRegistry::addPalette(CommandPaletteDesc desc) {
    _paletteDescs.push_back(std::move(desc));
}

void up::shell::CommandRegistry::addHotKey(CommandHotKeyDesc desc) {
    _hotKeyDescs.push_back(std::move(desc));
}

auto up::shell::CommandRegistry::commandAt(int index) const noexcept -> Command const* {
    if (index < 0 || index >= narrow_cast<int>(_commands.size())) {
        return nullptr;
    }

    return &_commands[index];
}

auto up::shell::CommandRegistry::execute(string_view input) -> CommandResult {
    for (auto& command : _commands) {
        if (command.command == input) {
            if (!_evaluate(command.whenId)) {
                return CommandResult::Predicate;
            }

            if (command.callback) {
                command.callback(input);
            }
            return CommandResult::Success;
        }
    }

    return CommandResult::NotFound;
}

auto up::shell::CommandRegistry::test(string_view input) -> CommandResult {
    for (auto& command : _commands) {
        if (command.command == input) {
            if (!_evaluate(command.whenId)) {
                return CommandResult::Predicate;
            }
            if (!_evaluate(command.enablementId)) {
                return CommandResult::Disabled;
            }
            return CommandResult::Success;
        }
    }

    return CommandResult::NotFound;
}
