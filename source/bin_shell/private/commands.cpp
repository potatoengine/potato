// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

auto up::shell::CommandProvider::addCommand(CommandDesc command) -> CommandId {
    auto const id = CommandId{hash_value(command.name)};
    _commands.push_back(Command{
        .id = id,
        .name = std::move(command.name),
        .predicate = std::move(command.predicate),
        .execute = std::move(command.execute)});
    return id;
}

bool up::shell::CommandProvider::execute(CommandId id, string_view input) {
    for (auto & command : _commands) {
        if (command.id == id) {
            if (command.predicate != nullptr) {
                auto const result = command.predicate();
                if (!result) {
                    return false;
                }
            }

            if (command.execute != nullptr) {
                command.execute(input);
            }

            return true;
        }
    }

    return false;
}

auto up::shell::CommandProvider::test(CommandId id) -> bool {
    for (auto& command : _commands) {
        if (command.id == id) {
            if (command.predicate == nullptr) {
                return true;
            }

            return command.predicate();
        }
    }

    return false;
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

    _rebuild();

    for (auto& command : _commands) {
        if (command.id == id) {
            return command.provider->execute(command.id, input);
        }
    }

    return false;
}

auto up::shell::CommandRegistry::test(string_view input) -> bool {
    auto const id = CommandId{hash_value(input)};

    _rebuild();

    for (auto& command : _commands) {
        if (command.id == id) {
            return command.provider->test(command.id);
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
        for (auto& command : provider->commands()) {
            _commands.push_back(CompiledCommand{
                .id = command.id,
                .provider = provider });
        }
    }
}
