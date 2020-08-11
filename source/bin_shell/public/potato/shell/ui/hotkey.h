// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "ui/action.h"

#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

namespace up::shell {
    class HotKeys {
    public:
        void bindActions(Actions& actions);

        [[nodiscard]] bool evaluateKey(int keysym, unsigned mods);

    private:
        struct HotKey {
            ActionId id = ActionId::None;
            int keycode = 0;
            unsigned mods = 0;
        };

        void _rebuild();
        [[nodiscard]] bool _compile(string_view input, int& out_key, unsigned& out_mods) const noexcept;
        [[nodiscard]] auto _stringify(int keycode, unsigned mods) const -> string;

        vector<HotKey> _hotKeys;
        Actions* _actions = nullptr;
        uint64 _actionsVersion = 0;
    };
} // namespace up::shell
