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

        if (!_isVisible(menu)) {
            continue;
        }

        if (menu.command.empty()) {
            if (ImGui::BeginMenu(menu.title.c_str())) {
                _drawMenu(menu.id);
                ImGui::EndMenu();
            }
        }
        else {
            bool const enabled = _isEnabled(menu);
            bool const checked = _isChecked(menu);
            if (ImGui::MenuItem(menu.title.c_str(), nullptr, checked, enabled)) {
                (void)_commands.execute(menu.command);
            }
        }
    }
}

auto up::shell::Menu::_isEnabled(MenuDesc const& desc) const noexcept -> bool {
    if (desc.command.empty()) {
        for (auto const& menu : _menus) {
            if (menu.id != desc.id) {
                continue;
            }
            if (_isEnabled(menu)) {
                return true;
            }
        }
        return false;
    }

    auto const result = _commands.test(desc.command);
    return result == CommandResult::Okay;
}

auto up::shell::Menu::_isChecked(MenuDesc const& desc) const noexcept -> bool {
    return desc.checkedId == tools::EvaluatorId{} ? false
                                                  : _commands.engine().evaluate(_commands.context(), desc.checkedId);
}

auto up::shell::Menu::_isVisible(MenuDesc const& desc) const noexcept -> bool {
    if (desc.command.empty()) {
        for (auto const& menu : _menus) {
            if (menu.parent != desc.id) {
                continue;
            }
            if (_isVisible(menu)) {
                return true;
            }
        }
        return false;
    }

    auto const result = _commands.test(desc.command);
    return result == CommandResult::Okay || result == CommandResult::Disabled;
}
