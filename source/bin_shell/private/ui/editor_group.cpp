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
}

void up::shell::EditorGroup::update(Renderer& renderer, float deltaTime) {
    for (auto it = _editors.begin(); it != _editors.end();) {
        if (it->get()->isClosed()) {
            if (it->get() == _active) {
                _active = nullptr;
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
        if (_editors[index]->isClosed()) {
            continue;
        }

        ImGui::SetNextWindowDockID(dockspaceId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_documentWindowClass);
        _editors[index]->render(renderer, deltaTime);
        _editors[index]->updateUi();
        if (!_editors[index]->isClosed() && _editors[index]->isActive()) {
            _active = _editors[index].get();
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

void up::shell::EditorGroup::open(box<Editor> editor) {
    _editors.push_back(std::move(editor));
}
