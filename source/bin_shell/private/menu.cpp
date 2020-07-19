// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "menu.h"
#include "commands.h"

#include <imgui.h>

void up::shell::Menu::addMenu(MenuDesc desc) {
    _menus.push_back(std::move(desc));
}

void up::shell::Menu::drawMenu(CommandRegistry& commands) const {
    if (ImGui::BeginMainMenuBar()) {
        _drawMenu(commands, {});
        ImGui::EndMainMenuBar();
    }
}

void up::shell::Menu::_drawMenu(CommandRegistry& commands, zstring_view parent) const {
    for (auto const& menu : _menus) {
        if (zstring_view{menu.parent} != parent) {
            continue;
        }

        if (!_isVisible(commands, menu)) {
            continue;
        }

        if (menu.command.empty()) {
            if (ImGui::BeginMenu(menu.title.c_str())) {
                _drawMenu(commands, menu.title);
                ImGui::EndMenu();
            }
        }
        else {
            bool enabled = _isEnabled(commands, menu);
            if (ImGui::MenuItem(menu.title.c_str(), nullptr, false, enabled)) {
                (void)commands.execute(menu.command);
            }
        }
    }
}

auto up::shell::Menu::_isEnabled(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool {
    if (desc.command.empty()) {
        for (auto const& menu : _menus) {
            if (menu.parent != desc.title) {
                continue;
            }
            if (_isVisible(commands, menu)) {
                return true;
            }
        }
        return false;
    }

    auto const result = commands.test(desc.command);
    return result == CommandResult::Okay;
}

auto up::shell::Menu::_isChecked(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool {
    return false;
}

auto up::shell::Menu::_isVisible(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool {
    if (desc.command.empty()) {
        for (auto const& menu : _menus) {
            if (menu.parent != desc.title) {
                continue;
            }
            if (_isVisible(commands, menu)) {
                return true;
            }
        }
        return false;
    }

    auto const result = commands.test(desc.command);
    return result == CommandResult::Okay || result == CommandResult::Disabled;
}
