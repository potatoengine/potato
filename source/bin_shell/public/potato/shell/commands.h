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
        string_view name;
        string title;
        CommandPredicate predicate;
        CommandExecuteDelegate execute;
    };

    /// @brief Provides a list of commands to the registry
    class CommandProvider {
    public:
        struct Command {
            CommandId id = {};
            string title;
            CommandPredicate predicate;
            CommandExecuteDelegate execute;
        };

        auto addCommand(CommandDesc desc) -> CommandId;

    private:
        vector<Command> _commands;

        friend class CommandRegistry;
    };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        struct Match {
            CommandId id = {};
            zstring_view title;
        };

        CommandRegistry();
        ~CommandRegistry();

        auto addProvider(CommandProvider* provider) -> bool;
        auto removeProvider(CommandProvider* provider) -> bool;

        [[nodiscard]] auto execute(CommandId id, string_view input) -> bool;
        [[nodiscard]] auto execute(string_view input) -> bool;
        [[nodiscard]] auto test(string_view input) -> bool;

        void match(string_view input, vector<Match>& inout_matches);

    private:
        struct CompiledCommand {
            CommandId id = {};
            CommandProvider* provider = nullptr;
            size_t index = 0;
        };

        void _rebuild();

        vector<CompiledCommand> _commands;
        vector<CommandProvider*> _providers;
        bool _dirty = false;
    };
} // namespace up::shell
