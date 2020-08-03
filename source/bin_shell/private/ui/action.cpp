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

void up::shell::ActionGroup::addAction(ActionDesc desc) {
    _actions.push_back(std::move(desc));
    ++_version;
}

auto up::shell::Actions::addGroup(ActionGroup* group) -> bool {
    if (group == nullptr) {
        return false;
    }

    if (contains(_groups, group, {}, &Record::group)) {
        return false;
    }

    _groups.push_back({.group = group});

    return true;
}

auto up::shell::Actions::removeGroup(ActionGroup const* group) -> bool {
    if (group == nullptr) {
        return false;
    }

    for (auto& record : _groups) {
        if (record.group == group) {
            record.group = nullptr;
            record.version = 0;
            return true;
        }
    }

    return false;
}

bool up::shell::Actions::refresh(uint64& lastVersion) noexcept {
    auto dirty = erase(_groups, nullptr, &Record::group) != 0;

    for (auto& group : _groups) {
        auto const currentVersion = group.group->_version;
        if (currentVersion != group.version) {
            group.version = currentVersion;
            dirty = true;
        }
    }

    if (dirty) {
        ++_version;
    }
    else if (_version == lastVersion) {
        return false;
    }

    lastVersion = _version;
    return true;
}

void up::shell::Actions::build(BuildCallback callback) {
    for (auto const& [groupIndex, group] : enumerate(_groups)) {
        for (auto const& [index, action] : enumerate(group.group->_actions)) {
            callback(idOf(groupIndex, index), action);
        }
    }
}

auto up::shell::Actions::isEnabled(ActionId id) -> bool {
    auto* group = _groups[groupOf(id)].group;
    auto& action = group->_actions[indexOf(id)];

    return action.enabled == nullptr || action.enabled();
}

auto up::shell::Actions::isChecked(ActionId id) -> bool {
    auto* group = _groups[groupOf(id)].group;
    auto& action = group->_actions[indexOf(id)];

    return action.checked != nullptr && action.checked();
}

void up::shell::Actions::invoke(ActionId id) {
    auto* group = _groups[groupOf(id)].group;
    auto& action = group->_actions[indexOf(id)];

    action.action();
}
