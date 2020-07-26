// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "ui/menu.h"
#include "commands.h"

#include "potato/spud/hash.h"

#include <imgui.h>

auto up::shell::Menu::addMenu(MenuDesc desc) -> MenuId {
    desc.id = MenuId{hash_combine(uint64{desc.parent}, hash_value(desc.title))};
    if (!desc.checked.empty()) {
        desc.checkedId = _commands.engine().compile(desc.checked);
    }
    _menus.push_back(std::move(desc));
    return desc.id;
}

void up::shell::Menu::drawMenu() const {
    if (ImGui::BeginMainMenuBar()) {
        _drawMenu({});
        ImGui::EndMainMenuBar();
    }
}

void up::shell::Menu::_drawMenu(MenuId parent) const {
    for (auto const& menu : _menus) {
        if (menu.parent != parent) {
            continue;
        }

        auto const state = _state(menu);

        if (!state.visible) {
            continue;
        }

        if (menu.command.empty()) {
            if (ImGui::BeginMenu(menu.title.c_str())) {
                _drawMenu(menu.id);
                ImGui::EndMenu();
            }
        }
        else {
            if (ImGui::MenuItem(menu.title.c_str(), nullptr, state.checked, state.enabled)) {
                (void)_commands.execute(menu.command);
            }
        }
    }
}

auto up::shell::Menu::_state(MenuDesc const& desc) const noexcept -> State {
    if (!desc.command.empty()) {
        auto const result = _commands.test(desc.command);
        return {
            .visible = result == CommandResult::Okay || result == CommandResult::Disabled,
            .enabled = result != CommandResult::Disabled,
            .checked = _commands.engine().evaluate(_commands.context(), desc.checkedId)};
    }

    for (auto const& menu : _menus) {
        if (menu.parent != desc.id) {
            continue;
        }

        auto const state = _state(menu);
        if (state.visible) {
            return {.visible = true};
        }
    }

    return {.visible = false};
}
