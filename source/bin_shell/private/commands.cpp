// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

bool up::shell::CommandRegistry::_evaluate(tools::EvaluatorId id) noexcept {
    return id == tools::EvaluatorId{} ? true : _context.evaluate(id);
}

void up::shell::CommandRegistry::registerCommand(Command command) {
    if (!command.enablement.empty()) {
        command.enablementId = _context.compile(command.enablement);
    }
    if (!command.when.empty()) {
        command.whenId = _context.compile(command.when);
    }
    _commands.push_back(std::move(command));
}

void up::shell::CommandRegistry::addPalette(CommandPaletteDesc const& desc) {
    _paletteDescs.push_back(desc);
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
            if (!_evaluate(command.whenId)) {
                return CommandResult::Predicate;
            }

            if (command.callback) {
                command.callback(input);
            }
            return CommandResult::Okay;
        }
    }

    return CommandResult::NotFound;
}

auto up::shell::CommandRegistry::test(string_view input) -> CommandResult {
    for (auto& command : _commands) {
        if (command.command == input) {
            if (!_evaluate(command.whenId)) {
                return CommandResult::Predicate;
            }
            if (!_evaluate(command.enablementId)) {
                return CommandResult::Disabled;
            }
            return CommandResult::Okay;
        }
    }

    return CommandResult::NotFound;
}
