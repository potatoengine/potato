// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::tools {
    enum class EvaluatorId : uint32 {};

    /// @brief Manages context variables and expressions
    class Evaluator {
    public:
        UP_TOOLS_API Evaluator();
        UP_TOOLS_API ~Evaluator();

        UP_TOOLS_API void set(string_view name, string_view value);
        void set(string_view name, char const* value) { set(name, string_view{value}); }
        UP_TOOLS_API void set(string_view name, bool value);
        UP_TOOLS_API void clear(string_view name);

        UP_TOOLS_API [[nodiscard]] EvaluatorId compile(string_view expr);

        UP_TOOLS_API [[nodiscard]] bool evaluate(EvaluatorId id) noexcept;

    private:
        using NameHash = uint64;
        using Value = uint64;
        using Index = uint32;
        using Version = uint32;

        enum class Op : char { Variable, Literal, Complement, Conjunction, Disjunction, Equality, Inequality };

        struct Variable {
            NameHash name = 0;
            Value value = 0;
        };

        struct Expr {
            NameHash hash = 0;
            Value memo = 0;
            Index arg0 = 0;
            Index arg1 = 0;
            Version version = ~Version{0};
            Op op = {};
        };

        static_assert(sizeof(Expr) == 32);

        void _set(NameHash name, Value value);
        Value _get(NameHash name, Value defaultValue = 0) const noexcept;
        [[nodiscard]] Index _add(Expr expr);
        [[nodiscard]] Index _add(Op op, Index arg1, Index arg0);
        [[nodiscard]] Index _add(Op op, Index arg);
        [[nodiscard]] Index _add(Op op, NameHash var);
        [[nodiscard]] Value _evaluate(Index index) noexcept;

        vector<Variable> _variables;
        vector<Expr> _exprs;
        Version _revision = 0;
    };
} // namespace up::tools
