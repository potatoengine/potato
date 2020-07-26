// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/tools/evaluator.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    using CommandDelegate = delegate<void(string_view input)>;

    struct Command {
        zstring_view command;
        zstring_view enablement;
        zstring_view when;
        tools::EvaluatorId enablementId = {};
        tools::EvaluatorId whenId = {};
        CommandDelegate callback;
    };

    enum CommandResult { Okay, NotFound, Predicate, Disabled, Argument };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        CommandRegistry();
        ~CommandRegistry();

        void registerCommand(Command command);

        auto engine() noexcept -> tools::EvalEngine& { return _engine; }
        auto context() noexcept -> tools::EvalContext& { return _context; }

        [[nodiscard]] auto execute(string_view input) -> CommandResult;
        [[nodiscard]] auto test(string_view input) -> CommandResult;

    private:
        struct Context {
            string name;
            string value;
        };

        [[nodiscard]] bool _evaluate(tools::EvaluatorId id) noexcept;

        tools::EvalEngine _engine;
        tools::EvalContext _context;
        vector<Command> _commands;
    };
} // namespace up::shell
