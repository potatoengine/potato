// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "ui/action.h"

#include "potato/spud/box.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor;

    class Menu;

    class EditorGroup {
    public:
        EditorGroup();
        ~EditorGroup();

        void update(Actions& actions, Renderer& renderer, float deltaTime);

        void closeAll() noexcept;
        void closeActive() noexcept;

        void open(box<Editor> editor);

    private:
        void _setActive(Actions& actions, Editor* editor);

        vector<box<Editor>> _editors;
        ActionGroup _actions;
        ImGuiWindowClass _documentWindowClass;
        Editor* _active = nullptr;
    };
} // namespace up::shell
