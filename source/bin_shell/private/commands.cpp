// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "commands.h"

#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

auto up::shell::CommandProvider::registerCommand(CommandDesc command) -> CommandId {
    auto const id = CommandId{hash_value(command.command)};
    _commands.push_back(Command{
        .id = id,
        .command = std::move(command.command),
        .enablement = std::move(command.enablement),
        .when = std::move(command.when),
        .callback = std::move(command.callback)});
    return id;
}

void up::shell::CommandProvider::execute(CommandId id, string_view input) const {
    for (auto const& command : _commands) {
        if (command.id == id) {
            if (command.callback != nullptr) {
                // TODO: no const_cast
                const_cast<CommandDelegate&>(command.callback)(input);
            }
            return;
        }
    }
}

up::shell::CommandRegistry::CommandRegistry() = default;

up::shell::CommandRegistry::~CommandRegistry() = default;

auto up::shell::CommandRegistry::addProvider(CommandProvider const* provider) -> bool {
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

auto up::shell::CommandRegistry::removeProvider(CommandProvider const* provider) -> bool {
    if (provider == nullptr) {
        return false;
    }

    erase(_providers, provider);
    _dirty = true;
    return true;
}

auto up::shell::CommandRegistry::execute(string_view input) -> CommandResult {
    auto const id = CommandId{hash_value(input)};

    _recompile();

    for (auto& command : _commands) {
        if (command.id == id) {
            if (!_evaluate(command.whenId)) {
                return CommandResult::Excluded;
            }
            if (!_evaluate(command.enablementId)) {
                return CommandResult::Disabled;
            }

            command.provider->execute(command.id, input);
            return CommandResult::Okay;
        }
    }

    return CommandResult::NotFound;
}

auto up::shell::CommandRegistry::test(string_view input) -> CommandResult {
    auto const id = CommandId{hash_value(input)};

    _recompile();

    for (auto& command : _commands) {
        if (command.id == id) {
            if (!_evaluate(command.whenId)) {
                return CommandResult::Excluded;
            }
            if (!_evaluate(command.enablementId)) {
                return CommandResult::Disabled;
            }
            return CommandResult::Okay;
        }
    }

    return CommandResult::NotFound;
}

void up::shell::CommandRegistry::_recompile() {
    if (!_dirty) {
        return;
    }
    _dirty = false;

    _commands.clear();

    for (auto* provider : _providers) {
        _compile(*provider);
    }
}

void up::shell::CommandRegistry::_compile(CommandProvider const& provider) {
    for (auto const& command : provider.commands()) {
        _commands.push_back(CompiledCommand{
            .id = command.id,
            .enablementId = _engine.compile(command.enablement),
            .whenId = _engine.compile(command.when),
            .provider = &provider});
    }
}

bool up::shell::CommandRegistry::_evaluate(tools::EvaluatorId id) noexcept {
    return id == tools::EvaluatorId{} ? true : _engine.evaluate(_context, id);
}
