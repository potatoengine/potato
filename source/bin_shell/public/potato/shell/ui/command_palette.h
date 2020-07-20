// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/vector.h"

struct ImGuiInputTextCallbackData;

namespace up::shell {
    class CommandRegistry;

    /// @brief Handles the command palette UI
    class CommandPalette {
    public:
        void show();
        void close();

        void update(CommandRegistry& registry);

    private:
        static int _callbackWrapper(ImGuiInputTextCallbackData* data);
        int _callback(ImGuiInputTextCallbackData* data);
        bool _execute(CommandRegistry& registry) const;
        void _updateMatches(CommandRegistry& registry);

        int _activeIndex = -1;
        char _input[128] = {0};
        vector<int> _matches;
        bool _open = false;
    };
} // namespace up::shell
