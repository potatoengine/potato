// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

void up::shell::CommandRegistry::registerCommand(Command command) {
    _commands.push_back(std::move(command));
}

void up::shell::CommandRegistry::addPalette(CommandPaletteDesc desc) {
    _paletteDescs.push_back(std::move(desc));
}

void up::shell::CommandRegistry::addHotKey(CommandHotKeyDesc desc) {
    _hotKeyDescs.push_back(std::move(desc));
}

auto up::shell::CommandRegistry::commandAt(int index) const noexcept -> Command const* {
    if (index < 0 || index >= narrow_cast<int>(_commands.size())) {
        return nullptr;
    }

    return &_commands[index];
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
