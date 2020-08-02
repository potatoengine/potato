// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "commands.h"
#include "ui/menu.h"

#include "potato/spud/box.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor;

    class EditorGroup {
    public:
        EditorGroup();
        ~EditorGroup();

        void update(CommandRegistry& commands, Menu& menu, Renderer& renderer, float deltaTime);

        void closeAll() noexcept;
        void closeActive() noexcept;

        void open(box<Editor> editor);

    private:
        void _setActive(CommandRegistry& commands, Menu& menu, Editor* editor);

        vector<box<Editor>> _editors;
        CommandProvider _commands;
        ImGuiWindowClass _documentWindowClass;
        Editor* _active = nullptr;
    };
} // namespace up::shell
