// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/box.h"
#include "potato/spud/vector.h"

#include <imgui.h>

namespace up {
    class Renderer;
}

namespace up::shell {
    class Editor;

    class EditorGroup {
    public:
        EditorGroup();

        EditorGroup(EditorGroup const&) = delete;
        EditorGroup& operator=(EditorGroup const&) = delete;

        void update(Renderer& renderer, float deltaTime);

        void closeAll() noexcept;
        void closeActive() noexcept;

        [[nodiscard]] auto active() const noexcept { return _active; }

        void open(box<Editor> editor);

    private:
        vector<box<Editor>> _editors;
        ImGuiWindowClass _documentWindowClass;
        Editor* _active = nullptr;
    };
} // namespace up::shell
