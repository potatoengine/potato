// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    class CommandRegistry;

    struct MenuItemDesc {
        string menu;
        char8_t const* icon = nullptr;
        string title;
        string command;
    };

    struct MenuDesc {
        char8_t const* icon = nullptr;
        string title;
        string parent;
        int priority = -1;
    };

    class Menu {
    public:
        void addMenuItem(MenuItemDesc desc);
        void addMenu(MenuDesc desc);

        void drawMenu(CommandRegistry& commands) const;

    private:
        void _drawMenu(CommandRegistry& commands, zstring_view parent) const;

        vector<MenuDesc> _menus;
        vector<MenuItemDesc> _items;
    };
} // namespace up::shell
