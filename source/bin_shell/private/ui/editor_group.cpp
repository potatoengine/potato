// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor_group.h"
#include "editor.h"

#include "potato/spud/sequence.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::EditorGroup::EditorGroup() {
    _documentWindowClass.ClassId = ImHashStr("EditorDocumentClass");
    _documentWindowClass.DockingAllowUnclassed = false;
    _documentWindowClass.DockingAlwaysTabBar = true;

    _commands.registerCommand(
        {.command = "potato.editors.closeActive", .enablement = "isEditorClosable", .callback = [this](string_view) {
             closeActive();
         }});
}

up::shell::EditorGroup::~EditorGroup() = default;

void up::shell::EditorGroup::update(CommandRegistry& commands, Renderer& renderer, float deltaTime) {
    commands.addProvider(&_commands);

    for (auto it = _editors.begin(); it != _editors.end();) {
        if (it->get()->isClosed()) {
            if (it->get() == _active) {
                _setActive(commands, nullptr);
            }
            it = _editors.erase(it);
        }
        else {
            ++it;
        }
    }

    for (auto index : sequence(_editors.size())) {
        _editors[index]->tick(deltaTime);
    }

    auto const dockspaceId = ImGui::GetID("EditorDockspace");
    ImGui::DockSpace(dockspaceId, {}, ImGuiDockNodeFlags_NoWindowMenuButton, &_documentWindowClass);

    for (auto index : sequence(_editors.size())) {
        Editor* editor = _editors[index].get();

        if (editor->isClosed()) {
            continue;
        }

        ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_documentWindowClass);
        editor->render(renderer, deltaTime);
        editor->updateUi();
        if (!editor->isClosed() && editor->isActive()) {
            _setActive(commands, editor);
        }
    }
}

void up::shell::EditorGroup::closeAll() noexcept {
    for (auto const& editor : _editors) {
        editor->close();
    }
}

void up::shell::EditorGroup::closeActive() noexcept {
    if (_active != nullptr) {
        _active->close();
    }
}

auto up::shell::EditorGroup::isActiveClosable() const noexcept -> bool {
    return _active != nullptr && _active->isClosable();
}

auto up::shell::EditorGroup::activeEditorClass() const noexcept -> zstring_view {
    return _active != nullptr ? _active->editorClass() : zstring_view{};
}

void up::shell::EditorGroup::open(box<Editor> editor) {
    _editors.push_back(std::move(editor));
}

void up::shell::EditorGroup::_setActive(CommandRegistry& commands, Editor* editor) {
    if (_active != nullptr) {
        commands.removeProvider(&_active->commands());
    }

    _active = editor;

    if (_active != nullptr) {
        commands.addProvider(&_active->commands());
    }
}
