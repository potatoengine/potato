// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

void up::shell::CommandRegistry::registerCommand(Command command) {
    _commands.push_back(std::move(command));
}

auto up::shell::CommandRegistry::commandAt(int index) const noexcept -> Command const* {
    if (index < 0 || index >= narrow_cast<int>(_commands.size())) {
        return nullptr;
    }

    return &_commands[index];
}

void up::shell::CommandRegistry::findMatches(
    zstring_view input,
    int& inout_currentIndex,
    vector<int>& out_matchIndices) {
    out_matchIndices.clear();

    int lastMatchIndex = -1;

    for (auto index : sequence(_commands.size())) {
        auto const& command = _commands[index];
        if (stringIndexOfNoCase(command.title.data(), command.title.size(), input.data(), input.size()) != -1) {
            int const matchIndex = narrow_cast<int>(index);

            if (matchIndex == inout_currentIndex || lastMatchIndex == -1) {
                lastMatchIndex = matchIndex;
            }

            out_matchIndices.push_back(matchIndex);
        }
    }

    inout_currentIndex = lastMatchIndex;
}

auto up::shell::CommandRegistry::execute(string_view input) -> CommandResult {
    for (auto& command : _commands) {
        if (command.command == input) {
            if (command.callback) {
                command.callback(input);
            }
            return CommandResult::Success;
        }
    }
    return CommandResult::NotFound;
}

auto up::shell::CommandRegistry::applyHotkey(int key, int mods) -> CommandResult {
    for (auto& command : _commands) {
        if (command.hotkey == key && (command.hotkeyMods & mods) != 0) {
            if (command.callback) {
                command.callback({});
            }
            return CommandResult::Success;
        }
    }
    return CommandResult::NotFound;
}
