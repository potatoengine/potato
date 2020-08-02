// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

auto up::shell::CommandProvider::addCommand(CommandDesc desc) -> CommandId {
    auto const id = CommandId{hash_value(desc.name)};
    _commands.push_back(Command{
        .id = id,
        .title = std::move(desc.title),
        .predicate = std::move(desc.predicate),
        .execute = std::move(desc.execute)});
    return id;
}

up::shell::CommandRegistry::CommandRegistry() = default;

up::shell::CommandRegistry::~CommandRegistry() = default;

auto up::shell::CommandRegistry::addProvider(CommandProvider* provider) -> bool {
    if (provider == nullptr) {
        return false;
    }

    if (contains(_providers, provider)) {
        return false;
    }

    _providers.push_back(provider);
    _dirty = true;
    return true;
}

auto up::shell::CommandRegistry::removeProvider(CommandProvider* provider) -> bool {
    if (provider == nullptr) {
        return false;
    }

    erase(_providers, provider);
    _dirty = true;
    return true;
}

auto up::shell::CommandRegistry::execute(string_view input) -> bool {
    auto const id = CommandId{hash_value(input)};
    return execute(id, input);
}

auto up::shell::CommandRegistry::execute(CommandId id, string_view input) -> bool {
    _rebuild();

    for (auto& command : _commands) {
        if (command.id == id) {
            auto& cmd = command.provider->_commands[command.index];

            if (cmd.predicate != nullptr) {
                if (!cmd.predicate()) {
                    return false;
                }
            }

            if (cmd.execute != nullptr) {
                cmd.execute(input);
            }

            return true;
        }
    }

    return false;
}

auto up::shell::CommandRegistry::test(string_view input) -> bool {
    auto const id = CommandId{hash_value(input)};

    _rebuild();

    for (auto& command : _commands) {
        if (command.id == id) {
            auto& cmd = command.provider->_commands[command.index];

            if (cmd.predicate == nullptr) {
                return true;
            }

            return cmd.predicate();
        }
    }

    return false;
}

void up::shell::CommandRegistry::_rebuild() {
    if (!_dirty) {
        return;
    }
    _dirty = false;

    _commands.clear();

    for (auto* provider : _providers) {
        for (auto const& [index, command] : enumerate(provider->_commands)) {
            _commands.push_back(CompiledCommand{.id = command.id, .provider = provider, .index = index});
        }
    }
}

void up::shell::CommandRegistry::match(string_view input, vector<Match>& inout_matches) {
    _rebuild();

    for (auto const& [index, command] : enumerate(_commands)) {
        zstring_view title = command.provider->_commands[command.index].title;

        if (stringIndexOfNoCase(title.data(), title.size(), input.data(), input.size()) == -1) {
            continue;
        }

        if (command.provider->_commands[command.index].predicate != nullptr) {
            if (!command.provider->_commands[command.index].predicate()) {
                continue;
            }
        }

        inout_matches.push_back({.id = command.id, .title = title});
    }
}
