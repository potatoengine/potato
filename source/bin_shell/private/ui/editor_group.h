// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "commands.h"

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

        EditorGroup(EditorGroup const&) = delete;
        EditorGroup& operator=(EditorGroup const&) = delete;

        void update(CommandRegistry& commands, Renderer& renderer, float deltaTime);

        void closeAll() noexcept;
        void closeActive() noexcept;

        [[nodiscard]] auto hasActive() const noexcept { return _active != nullptr; }
        [[nodiscard]] auto isActiveClosable() const noexcept -> bool;
        [[nodiscard]] auto activeEditorClass() const noexcept -> zstring_view;

        void open(box<Editor> editor);

    private:
        void _setActive(CommandRegistry& commands, Editor* editor);

        vector<box<Editor>> _editors;
        CommandProvider _commands;
        ImGuiWindowClass _documentWindowClass;
        Editor* _active = nullptr;
    };
} // namespace up::shell
