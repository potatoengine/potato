// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    using CommandDelegate = delegate<void(string_view input)>;

    struct Command {
        zstring_view command;
        CommandDelegate callback;
    };

    struct CommandPaletteDesc {
        zstring_view title;
        zstring_view command;
    };

    struct CommandHotKeyDesc {
        int key = 0;
        unsigned mods = 0;
        zstring_view command;
    };

    enum CommandResult { Success, NotFound, Argument };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        auto commandAt(int index) const noexcept -> Command const*;
        auto paletteDescs() const noexcept -> view<CommandPaletteDesc> { return _paletteDescs; }
        auto hotKeyDescs() const noexcept -> view<CommandHotKeyDesc> { return _hotKeyDescs; }

        void registerCommand(Command command);
        void addPalette(CommandPaletteDesc desc);
        void addHotKey(CommandHotKeyDesc desc);

        auto execute(string_view input) -> CommandResult;

    private:
        vector<Command> _commands;
        vector<CommandPaletteDesc> _paletteDescs;
        vector<CommandHotKeyDesc> _hotKeyDescs;
    };
} // namespace up::shell
