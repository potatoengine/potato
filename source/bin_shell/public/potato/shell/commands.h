// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    enum class CommandId : uint64 {};

    using CommandPredicate = delegate<bool()>;
    using CommandExecuteDelegate = delegate<void(string_view input)>;

    struct CommandDesc {
        string name;
        CommandPredicate predicate;
        CommandExecuteDelegate execute;
    };

    /// @brief Provides a list of commands to the registry
    class CommandProvider {
    public:
        struct Command {
            CommandId id = {};
            string name;
            CommandPredicate predicate;
            CommandExecuteDelegate execute;
        };

        auto addCommand(CommandDesc command) -> CommandId;

        [[nodiscard]] auto commands() const noexcept -> view<Command> { return _commands; }

        bool execute(CommandId id, string_view input);
        auto test(CommandId id) -> bool;

    private:
        vector<Command> _commands;
    };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        CommandRegistry();
        ~CommandRegistry();

        auto addProvider(CommandProvider* provider) -> bool;
        auto removeProvider(CommandProvider* provider) -> bool;

        [[nodiscard]] auto execute(string_view input) -> bool;
        [[nodiscard]] auto test(string_view input) -> bool;

    private:
        struct CompiledCommand {
            CommandId id = {};
            CommandProvider* provider = nullptr;
        };

        void _rebuild();

        vector<CompiledCommand> _commands;
        vector<CommandProvider*> _providers;
        bool _dirty = false;
    };
} // namespace up::shell
