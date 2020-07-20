// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::shell {
    class CommandRegistry;

    struct HotKeyDesc {
        int key = 0;
        unsigned mods = 0;
        string command;
    };

    class HotKeys {
    public:
        void addHotKey(HotKeyDesc desc);

        bool evaluateKey(CommandRegistry& commands, int keysym, unsigned mods);

    private:
        vector<HotKeyDesc> _hotKeys;
    };
} // namespace up::shell
