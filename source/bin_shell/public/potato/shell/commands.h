// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    using CommandDelegate = delegate<void(string_view input)>;
    enum class CommandContextId : uint64 {};

    struct Command {
        zstring_view command;
        zstring_view enablement;
        zstring_view when;
        CommandContextId enablementId = {};
        CommandContextId whenId = {};
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

    /// @brief Manages context variables and expressions
    class CommandContext {
    public:
        void set(string_view name, string_view value);
        void set(string_view name, bool value);
        [[nodiscard]] string_view get(string_view name, string_view defaultValue = {}) const noexcept;
        void clear(string_view name);

        [[nodiscard]] CommandContextId variable(string_view name);
        [[nodiscard]] CommandContextId negate(CommandContextId exprId);
        [[nodiscard]] CommandContextId conjuction(CommandContextId leftId, CommandContextId rightId);
        [[nodiscard]] CommandContextId disjunction(CommandContextId leftId, CommandContextId rightId);

        [[nodiscard]] CommandContextId compile(string_view when);

        [[nodiscard]] bool evaluate(CommandContextId id) noexcept { return _evaluate(uint64{id}); }

    private:
        struct Value {
            uint64 var = 0;
            string value;
        };

        struct Memo {
            uint64 id = 0;
            bool result = false;
        };

        enum class Op : char { Variable, Negate, Conjunction, Disjunction };

        struct Expr {
            Op op = {};
            uint64 id = 0;
            uint64 arg0 = 0;
            uint64 arg1 = 0;
        };

        string_view _get(uint64 var, string_view defaultValue = {}) const noexcept;
        CommandContextId _add(Expr expr);
        [[nodiscard]] bool _evaluate(uint64 id) noexcept;

        vector<Value> _values;
        vector<Memo> _memos;
        vector<Expr> _exprs;
    };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        auto commandAt(int index) const noexcept -> Command const*;
        auto paletteDescs() const noexcept -> view<CommandPaletteDesc> { return _paletteDescs; }
        auto hotKeyDescs() const noexcept -> view<CommandHotKeyDesc> { return _hotKeyDescs; }

        void registerCommand(Command command);

        auto context() noexcept -> CommandContext& { return _context; }

        void addPalette(CommandPaletteDesc desc);
        void addHotKey(CommandHotKeyDesc desc);

        [[nodiscard]] auto execute(string_view input) -> CommandResult;
        [[nodiscard]] auto test(string_view input) -> CommandResult;

    private:
        struct Context {
            string name;
            string value;
        };

        [[nodiscard]] bool _evaluate(CommandContextId id) noexcept;

        CommandContext _context;
        vector<Command> _commands;
        vector<CommandPaletteDesc> _paletteDescs;
        vector<CommandHotKeyDesc> _hotKeyDescs;
    };
} // namespace up::shell
