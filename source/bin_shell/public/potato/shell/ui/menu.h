// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "ui/action.h"

#include "potato/spud/delegate.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    /// @brief Contains the active state for rendering and handling a menu
    class Menu {
    public:
        void bindActions(Actions& actions);

        void drawMenu();

    private:
        static constexpr auto noGroup = ~size_t{0};

        struct CompiledItem {
            uint64 hash = 0;
            ActionId id = ActionId::None;
            size_t stringIndex = 0;
            size_t childIndex = 0;
            size_t siblingIndex = 0;
        };

        void _rebuild();
        void _drawMenu(size_t index);
        auto _findIndexByHash(uint64 hash) const noexcept -> size_t;
        void _insertChild(size_t parentIndex, size_t childIndex) noexcept;
        auto _createMenu(string_view menu) -> size_t;
        auto _recordString(string_view string) -> size_t;

        vector<CompiledItem> _items;
        vector<string> _strings;
        Actions* _actions = nullptr;
        uint64 _actionsVersion = 0;
    };
} // namespace up::shell
