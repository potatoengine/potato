// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/delegate.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

struct ImGuiInputTextCallbackData;

namespace up::shell {
    using CommandDelegate = delegate<void(string_view input)>;

    struct Command {
        zstring_view title;
        zstring_view command;
        CommandDelegate callback;
    };

    enum CommandResult { Success, NotFound, Argument };

    /// @brief Manages the list of all known commands in the system
    class CommandRegistry {
    public:
        auto commandAt(int index) const noexcept -> Command const*;

        void registerCommand(Command command);

        void findMatches(zstring_view input, int& inout_currentIndex, vector<int>& out_matchIndices);

        auto execute(string_view input) -> CommandResult;

    private:
        vector<Command> _commands;
    };
} // namespace up::shell
