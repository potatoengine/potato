// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

void up::shell::CommandRegistry::registerCommand(Command command) {
    _commands.push_back(std::move(command));
}

void up::shell::CommandRegistry::setContext(string_view name, string_view value) {
    for (auto& context : _context) {
        if (string_view{context.name} == name) {
            context.value = string{value};
            return;
        }
    }
    _context.push_back({.name = std::move(name), .value = std::move(value)});
}

auto up::shell::CommandRegistry::getContext(string_view name, string_view defaultValue) const -> string_view {
    for (auto& context : _context) {
        if (string_view{context.name} == name) {
            return context.value;
        }
    }
    return defaultValue;
}

void up::shell::CommandRegistry::clearContext(string_view name) {
    for (auto it = _context.begin(); it != _context.end(); ++it) {
        if (string_view{it->name} == name) {
            _context.erase(it);
            return;
        }
    }
}

auto up::shell::CommandRegistry::evaluate(string_view when) const noexcept -> bool {
    return getContext(when) == "YES"_sv;
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
            if (!command.when.empty() && !evaluate(command.when)) {
                return CommandResult::Predicate;
            }

            if (command.callback) {
                command.callback(input);
            }
            return CommandResult::Success;
        }
    }

    return CommandResult::NotFound;
}

auto up::shell::CommandRegistry::test(string_view input) -> CommandResult {
    for (auto& command : _commands) {
        if (command.command == input) {
            if (!command.when.empty() && !evaluate(command.when)) {
                return CommandResult::Predicate;
            }
            if (!command.enablement.empty() && !evaluate(command.enablement)) {
                return CommandResult::Disabled;
            }
            return CommandResult::Success;
        }
    }

    return CommandResult::NotFound;
}
