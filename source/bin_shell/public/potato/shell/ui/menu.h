// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/tools/evaluator.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    class CommandRegistry;

    enum class MenuId : uint64 {};

    struct MenuDesc {
        MenuId id = {};
        MenuId parent = {};
        char8_t const* icon = nullptr;
        string title;
        string command;
        string checked;
        tools::EvaluatorId checkedId;
        int priority = -1;
    };

    class Menu {
    public:
        Menu(CommandRegistry& commands) : _commands(commands) {}

        MenuId addMenu(MenuDesc desc);

        void drawMenu() const;

    private:
        void _drawMenu(MenuId parent) const;
        auto _isVisible(MenuDesc const& desc) const noexcept -> bool;
        auto _isEnabled(MenuDesc const& desc) const noexcept -> bool;
        auto _isChecked(MenuDesc const& desc) const noexcept -> bool;

        vector<MenuDesc> _menus;
        CommandRegistry& _commands;
    };
} // namespace up::shell
