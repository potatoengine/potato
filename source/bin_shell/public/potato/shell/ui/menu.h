// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/tools/evaluator.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/delegate.h"

namespace up::shell {
    class CommandRegistry;

    using MenuPredicate = delegate<bool()>;
    using MenuAction = delegate<void()>;

    struct MenuItemDesc {
        char8_t const* icon = nullptr;
        string title;
        MenuPredicate enabled;
        MenuPredicate checked;
        MenuAction action;
        int priority = -1;
    };

    struct MenuCommandDesc {
        char8_t const* icon = nullptr;
        string title;
        string command;
        int priority = -1;
    };

    /// @brief Contains a set of menu items to be displayed
    class MenuProvider {
    public:
        struct MenuItem {
            uint64 hash = 0;
            uint64 parentHash = 0;
            char8_t const* icon = nullptr;
            string title;
            string command;
            MenuPredicate enabled;
            MenuPredicate checked;
            MenuAction action;
            int priority = -1;
        };

        void addMenuItem(MenuItemDesc desc);
        void addMenuCommand(CommandRegistry& commands, MenuCommandDesc desc);

    private:
        vector<MenuItem> _items;

        friend class Menu;
    };

    /// @brief Contains the active state for rendering and handling a menu
    class Menu {
    public:
        auto addProvider(MenuProvider* provider) -> bool;
        auto removeProvider(MenuProvider* provider) -> bool;

        void drawMenu();

    private:
        struct MenuItem {
            size_t providerIndex = 0;
            size_t itemIndex = 0;
            size_t childIndex = 0;
            size_t siblingIndex = 0;
        };

        void _rebuild();
        void _drawMenu(size_t index);
        auto _findIndexByHash(uint64 hash) const noexcept -> size_t;
        void _insertChild(size_t parentIndex, size_t childIndex) noexcept;

        vector<MenuItem> _items;
        vector<MenuProvider*> _providers;
        bool _dirty = false;
    };
} // namespace up::shell
