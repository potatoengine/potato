// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/tools/evaluator.h"
#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    enum class CommandId : uint64 {};
    using CommandDelegate = delegate<void(string_view input)>;

    struct CommandDesc {
        string command;
        string enablement;
        string when;
        CommandDelegate callback;
    };

    enum CommandResult { Okay, NotFound, Excluded, Disabled, Argument };

    /// @brief Provides a list of commands to the registry
    class CommandProvider {
    public:
        struct Command {
            CommandId id = {};
            string command;
            string enablement;
            string when;
            CommandDelegate callback;
        };

        auto registerCommand(CommandDesc command) -> CommandId;

        [[nodiscard]] auto commands() const noexcept -> view<Command> { return _commands; }

        void execute(CommandId id, string_view input) const;

    private:
        vector<Command> _commands;
    };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        CommandRegistry();
        ~CommandRegistry();

        auto addProvider(CommandProvider const* provider) -> bool;
        auto removeProvider(CommandProvider const* provider) -> bool;

        auto engine() noexcept -> tools::EvalEngine& { return _engine; }
        auto context() noexcept -> tools::EvalContext& { return _context; }

        [[nodiscard]] auto execute(string_view input) -> CommandResult;
        [[nodiscard]] auto test(string_view input) -> CommandResult;

    private:
        struct CompiledCommand {
            CommandId id = {};
            tools::EvaluatorId enablementId = {};
            tools::EvaluatorId whenId = {};
            CommandProvider const* provider = nullptr;
        };

        void _recompile();
        void _compile(CommandProvider const& provider);
        [[nodiscard]] bool _evaluate(tools::EvaluatorId id) noexcept;

        tools::EvalEngine _engine;
        tools::EvalContext _context;
        vector<CompiledCommand> _commands;
        vector<CommandProvider const*> _providers;
        bool _dirty = false;
    };
} // namespace up::shell
