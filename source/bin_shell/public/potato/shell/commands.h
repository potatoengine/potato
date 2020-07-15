// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

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

    enum CommandResult { Success, NotFound, Predicate, Disabled, Argument };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        auto commandAt(int index) const noexcept -> Command const*;
        auto paletteDescs() const noexcept -> view<CommandPaletteDesc> { return _paletteDescs; }
        auto hotKeyDescs() const noexcept -> view<CommandHotKeyDesc> { return _hotKeyDescs; }

        void registerCommand(Command command);

        void setContext(string_view name, string_view value);
        [[nodiscard]] auto getContext(string_view name, string_view defaultValue = {}) const -> string_view;
        void clearContext(string_view name);

        [[nodiscard]] bool evaluate(string_view when) const noexcept;

        void addPalette(CommandPaletteDesc desc);
        void addHotKey(CommandHotKeyDesc desc);

        [[nodiscard]] auto execute(string_view input) -> CommandResult;
        [[nodiscard]] auto test(string_view input) -> CommandResult;

    private:
        struct Context {
            string name;
            string value;
        };

        vector<Context> _context;
        vector<Command> _commands;
        vector<CommandPaletteDesc> _paletteDescs;
        vector<CommandHotKeyDesc> _hotKeyDescs;
    };
} // namespace up::shell
