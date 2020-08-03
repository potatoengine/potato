// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/menu.h"
#include "ui/action.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"
#include "potato/spud/sequence.h"

#include <imgui.h>

void up::shell::Menu::bindActions(Actions& actions) {
    _actions = &actions;
}

void up::shell::Menu::drawMenu() {
    _rebuild();

    if (ImGui::BeginMainMenuBar()) {
        if (!_items.empty()) {
            _drawMenu(_items.front().childIndex);
        }
        ImGui::EndMainMenuBar();
    }
}

void up::shell::Menu::_rebuild() {
    if (_actions == nullptr || !_actions->refresh(_actionsVersion)) {
        return;
    }

    _items.clear();
    _items.emplace_back();

    _actions->build([this](auto const id, auto const& action) {
        if (action.menu.empty()) {
            return;
        }

        auto parentIndex = size_t{0};

        auto title = string_view{action.menu};
        auto const hash = hash_value(action.menu);
        auto parentHash = uint64{0};

        auto const lastSep = title.find_last_of("\\");
        if (lastSep != zstring_view::npos) {
            auto const menuTitle = title.substr(0, lastSep);
            title = title.substr(lastSep + 1);

            parentIndex = _createMenu(menuTitle);
        }

        auto const titleIndex = _recordString(title);

        _insertChild(parentIndex, _items.size());
        _items.push_back({.hash = hash, .id = id, .stringIndex = titleIndex});
    });
}

void up::shell::Menu::_drawMenu(size_t index) {
    while (index != 0) {
        auto const& item = _items[index];

        if (item.id != ActionId::None) {
            auto const checked = _actions->isChecked(item.id);
            auto const enabled = _actions->isEnabled(item.id);
            auto const hotKey = _actions->actionAt(item.id).hotKey;

            if (ImGui::MenuItem(_strings[item.stringIndex].c_str(), hotKey.c_str(), checked, enabled)) {
                _actions->invoke(item.id);
            }
        }
        else {
            if (ImGui::BeginMenu(_strings[item.stringIndex].c_str())) {
                _drawMenu(item.childIndex);
                ImGui::EndMenu();
            }
        }

        index = item.siblingIndex;
    }
}

auto up::shell::Menu::_findIndexByHash(uint64 hash) const noexcept -> size_t {
    if (hash == 0) {
        return 0;
    }

    for (auto const& [index, item] : enumerate(_items)) {
        if (item.hash == hash) {
            return index;
        }
    }

    return 0;
}

void up::shell::Menu::_insertChild(size_t parentIndex, size_t childIndex) noexcept {
    auto& parent = _items[parentIndex];

    if (parent.childIndex == 0) {
        parent.childIndex = childIndex;
        return;
    }

    auto index = parent.childIndex;
    while (_items[index].siblingIndex != 0) {
        index = _items[index].siblingIndex;
    }

    _items[index].siblingIndex = childIndex;
}

auto up::shell::Menu::_createMenu(string_view title) -> size_t {
    if (title.empty()) {
        return 0;
    }

    auto const hash = hash_value(title);
    auto const index = _findIndexByHash(hash);
    if (index != 0) {
        return index;
    }

    auto parentIndex = size_t{0};

    auto const lastSep = title.find_last_of("\\");
    if (lastSep != zstring_view::npos) {
        parentIndex = _createMenu(title.substr(0, lastSep));
        title = title.substr(lastSep + 1);
    }

    auto const titleIndex = _recordString(title);
    auto const newIndex = _items.size();
    _insertChild(parentIndex, newIndex);
    _items.push_back({.hash = hash, .stringIndex = titleIndex});
    return newIndex;
}

auto up::shell::Menu::_recordString(string_view string) -> size_t {
    for (auto const& [index, str] : enumerate(_strings)) {
        if (string_view{str} == string) {
            return index;
        }
    }

    auto const index = _strings.size();
    _strings.emplace_back(string);
    return index;
}
