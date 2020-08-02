// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/menu.h"
#include "commands.h"

#include "potato/spud/enumerate.h"
#include "potato/spud/erase.h"
#include "potato/spud/find.h"
#include "potato/spud/hash.h"

#include <imgui.h>

void up::shell::MenuProvider::addMenuItem(MenuItemDesc desc) {
    auto const hash = hash_value(desc.title);

    // Don't allow duplicates
    //
    for (auto const& item : _items) {
        if (item.hash == hash) {
            return;
        }
    }

    auto title = std::move(desc.title);
    auto parentHash = uint64{0};

    auto const lastSep = title.find_last_of("\\");
    if (lastSep != zstring_view::npos) {
        auto const menuTitle = title.substr(0, lastSep);
        parentHash = hash_value(menuTitle);

        // add the parent, in case it doesn't exist yet
        //
        addMenuItem({.title = string(menuTitle)});

        title = title.substr(lastSep + 1);
    }

    _items.push_back(MenuItem{
        .hash = hash,
        .parentHash = parentHash,
        .icon = desc.icon,
        .title = std::move(title),
        .enabled = std::move(desc.enabled),
        .checked = std::move(desc.checked),
        .action = std::move(desc.action),
        .priority = desc.priority});
}

void up::shell::MenuProvider::addMenuCommand(CommandRegistry& commands, MenuCommandDesc desc) {
    addMenuItem({
        .icon = desc.icon,
        .title = std::move(desc.title),
        .enabled = [reg = &commands, cmd = desc.command]{ return reg->test(cmd); },
        .action = [reg = &commands, cmd = desc.command]{ (void)reg->execute(cmd); },
        .priority = desc.priority
    });
}

auto up::shell::Menu::addProvider(MenuProvider* provider) -> bool {
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

auto up::shell::Menu::removeProvider(MenuProvider* provider) -> bool {
    if (provider == nullptr) {
        return false;
    }

    erase(_providers, provider);
    _dirty = true;
    return true;
}

void up::shell::Menu::drawMenu() {
    _rebuild();

    if (ImGui::BeginMainMenuBar()) {
        _drawMenu(_items.front().childIndex);
        ImGui::EndMainMenuBar();
    }
}

void up::shell::Menu::_rebuild() {
    if (!_dirty) {
        return;
    }
    _dirty = false;

    _items.clear();
    _items.emplace_back();
    for (auto const& [providerIndex, provider] : enumerate(_providers)) {
        for (auto const& [index, item] : enumerate(provider->_items)) {
            auto const parentIndex = _findIndexByHash(item.parentHash);

            _insertChild(parentIndex, _items.size());
            _items.push_back({.providerIndex = providerIndex, .itemIndex = index});
        }
    }
}

void up::shell::Menu::_drawMenu(size_t index) {
    while (index != 0) {
        auto const& item = _items[index];

        auto* provider = _providers[item.providerIndex];
        auto& itemData = provider->_items[item.itemIndex];

        if (itemData.action == nullptr) {
            if (ImGui::BeginMenu(itemData.title.c_str())) {
                _drawMenu(item.childIndex);
                ImGui::EndMenu();
            }
        }
        else {
            auto const checked = itemData.checked == nullptr ? false : itemData.checked();
            auto const enabled = itemData.enabled == nullptr ? true : itemData.enabled();

            if (ImGui::MenuItem(itemData.title.c_str(), nullptr, checked, enabled)) {
                itemData.action();
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
        if (index == 0) {
            continue;
        }

        auto const* provider = _providers[item.providerIndex];
        auto const& itemData = provider->_items[item.itemIndex];

        if (itemData.hash == hash) {
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
