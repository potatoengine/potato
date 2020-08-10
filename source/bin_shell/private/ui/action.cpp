// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/action.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/numeric_util.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/utility.h"

up::shell::ActionGroup::~ActionGroup() {
    if (_owner != nullptr) {
        _owner->removeGroup(this);
    }
}

void up::shell::ActionGroup::addAction(ActionDesc desc) {
    _actions.push_back(std::move(desc));

    if (_owner != nullptr) {
        _owner->invalidate();
    }
}

auto up::shell::Actions::addGroup(ActionGroup* group) -> bool {
    if (group == nullptr) {
        return false;
    }
    if (group->_owner != nullptr) {
        return false;
    }

    group->_owner = this;
    _groups.push_back(group);
    ++_version;

    return true;
}

auto up::shell::Actions::removeGroup(ActionGroup const* group) -> bool {
    if (group == nullptr) {
        return false;
    }
    if (group->_owner != this) {
        return false;
    }

    for (auto& record : _groups) {
        if (record == group) {
            record->_owner = nullptr;
            record = nullptr;
            ++_version;
            return true;
        }
    }

    return false;
}

bool up::shell::Actions::refresh(uint64& lastVersion) noexcept {
    erase(_groups, nullptr);

    auto const changed = lastVersion != _version;
    lastVersion = _version;
    return changed;
}

void up::shell::Actions::build(BuildCallback callback) {
    for (auto const& [groupIndex, group] : enumerate(_groups)) {
        for (auto const& [index, action] : enumerate(group->_actions)) {
            callback(idOf(groupIndex, index), action);
        }
    }
}

auto up::shell::Actions::isEnabled(ActionId id) -> bool {
    auto* group = _groups[groupOf(id)];
    auto& action = group->_actions[indexOf(id)];

    return action.enabled == nullptr || action.enabled();
}

auto up::shell::Actions::isChecked(ActionId id) -> bool {
    auto* group = _groups[groupOf(id)];
    auto& action = group->_actions[indexOf(id)];

    return action.checked != nullptr && action.checked();
}

void up::shell::Actions::invoke(ActionId id) {
    auto* group = _groups[groupOf(id)];
    auto& action = group->_actions[indexOf(id)];

    action.action();
}
