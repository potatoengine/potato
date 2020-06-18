// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "document.h"

#include "potato/format/format.h"
#include "potato/spud/string_writer.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::Document::Document(zstring_view className) {
    _windowClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _windowClass.DockingAllowUnclassed = false;
    _windowClass.DockingAlwaysTabBar = false;
}

void up::shell::Document::render(Renderer& renderer) {
    ImGui::PushID(this);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const open = ImGui::Begin(displayName().c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar(1);

    auto const dockId = ImGui::GetID("ContentDockspace");

    if (_documentId.empty()) {
        string_writer tmp;
        format_append(tmp, "Document##{}", displayName());
        _documentId = tmp.to_string();
    }

    if (ImGui::DockBuilderGetNode(dockId) == nullptr) {
        ImGui::DockBuilderRemoveNode(dockId);
        ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
        buildDockSpace(dockId, _documentId);
        ImGui::DockBuilderFinish(dockId);
    }

    ImGui::DockSpace(dockId, {}, ImGuiDockNodeFlags_None, &_windowClass);

    if (open) {
        ImGui::SetNextWindowDockID(dockId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_windowClass);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        auto const contentOpen = ImGui::Begin(_documentId.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
        ImGui::PopStyleVar(1);

        if (contentOpen) {
            renderContent(renderer);
        }

        ImGui::End();
    }

    renderMenu();

    ImGui::End();
    ImGui::PopID();
}
