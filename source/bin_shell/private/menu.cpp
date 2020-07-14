// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "menu.h"
#include "commands.h"

#include <imgui.h>

void up::shell::Menu::addMenuItem(MenuItemDesc desc) {
    _items.push_back(desc);
}

void up::shell::Menu::addMenu(MenuDesc desc) {
    _menus.push_back(desc);
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

        if (ImGui::BeginMenu(menu.title.c_str())) {
            _drawMenu(commands, menu.title);
            ImGui::EndMenu();
        }
    }

    for (auto const& item : _items) {
        if (zstring_view{item.menu} != parent) {
            continue;
        }

        if (!commands.isExecutable(item.command)) {
            continue;
        }

        if (ImGui::MenuItem(item.title.c_str())) {
            (void)commands.execute(item.command);
        }
    }
}
