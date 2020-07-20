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

    struct CommandPaletteDesc {
        zstring_view title;
        zstring_view command;
    };

    enum CommandResult { Okay, NotFound, Predicate, Disabled, Argument };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        auto commandAt(int index) const noexcept -> Command const*;
        auto paletteDescs() const noexcept -> view<CommandPaletteDesc> { return _paletteDescs; }

        void registerCommand(Command command);

        auto context() noexcept -> tools::Evaluator& { return _context; }

        void addPalette(CommandPaletteDesc const& desc);

        [[nodiscard]] auto execute(string_view input) -> CommandResult;
        [[nodiscard]] auto test(string_view input) -> CommandResult;

    private:
        struct Context {
            string name;
            string value;
        };

        [[nodiscard]] bool _evaluate(tools::EvaluatorId id) noexcept;

        tools::Evaluator _context;
        vector<Command> _commands;
        vector<CommandPaletteDesc> _paletteDescs;
    };
} // namespace up::shell
