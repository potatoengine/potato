// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::tools {
    namespace _detail::evaluator {
        using NameHash = uint64;
        using Value = uint64;
        using Version = uint32;
        using Index = uint32;
    } // namespace _detail::evaluator

    enum class EvaluatorId : _detail::evaluator::Index {};
    enum class VariableId : _detail::evaluator::NameHash {};

    /// @brief Variable context for evaluator
    class EvalContext {
    public:
        using NameHash = _detail::evaluator::NameHash;
        using Value = _detail::evaluator::Value;
        using Version = _detail::evaluator::Version;

        UP_TOOLS_API void set(string_view name, string_view value);
        void set(string_view name, char const* value) { set(name, string_view{value}); }
        UP_TOOLS_API void set(string_view name, bool value);
        UP_TOOLS_API void clear(string_view name);

        Value getHashedValue(NameHash name, Value defaultValue = 0) const noexcept;

        Version version() const noexcept;

    private:
        struct Variable {
            NameHash name = {};
            Value value = 0;
        };

        void _set(NameHash name, Value value);

        vector<Variable> _variables;
        uint32 _revision = 0;
    };

    /// @brief Compiles and evaluates and expressions
    class EvalEngine {
    public:
        UP_TOOLS_API EvalEngine();
        UP_TOOLS_API ~EvalEngine();

        UP_TOOLS_API EvaluatorId compile(string_view expr);

        UP_TOOLS_API bool evaluate(EvalContext& context, EvaluatorId id) noexcept;

    private:
        using NameHash = _detail::evaluator::NameHash;
        using Index = _detail::evaluator::Index;
        using Value = _detail::evaluator::Value;
        using Version = _detail::evaluator::Version;

        enum class Op : char { Variable, Literal, Complement, Conjunction, Disjunction, Equality, Inequality };

        struct Expr {
            NameHash hash = 0;
            Value memo = 0;
            Index arg0 = 0;
            Index arg1 = 0;
            Version version = ~Version{0};
            Op op = {};
        };

        static_assert(sizeof(Expr) == 32);

        [[nodiscard]] Index _add(Expr expr);
        [[nodiscard]] Index _add(Op op, Index arg1, Index arg0);
        [[nodiscard]] Index _add(Op op, Index arg);
        [[nodiscard]] Index _add(Op op, NameHash var);
        [[nodiscard]] Value _evaluate(EvalContext& context, Version revision, Index index) noexcept;

        vector<Expr> _exprs;
    };
} // namespace up::tools
