// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    class CommandRegistry;

    struct MenuDesc {
        string parent;
        char8_t const* icon = nullptr;
        string title;
        string command;
        int priority = -1;
    };

    class Menu {
    public:
        void addMenu(MenuDesc desc);

        void drawMenu(CommandRegistry& commands) const;

    private:
        void _drawMenu(CommandRegistry& commands, zstring_view parent) const;
        auto _isVisible(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool;
        auto _isEnabled(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool;
        auto _isChecked(CommandRegistry& commands, MenuDesc const& desc) const noexcept -> bool;

        vector<MenuDesc> _menus;
    };
} // namespace up::shell
