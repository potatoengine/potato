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
    if (_title.empty()) {
        string_writer tmp;
        format_append(tmp, "{}##{}", displayName(), this);
        _title = std::move(tmp).to_string();
    }

    ImGui::PushID(this);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const open = ImGui::Begin(_title.c_str(), &_wantOpen, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar(1);

    if (_documentId.empty()) {
        string_writer tmp;
        format_append(tmp, "Document##{}", this);
        _documentId = std::move(tmp).to_string();
    }

    auto const dockSpaceId = ImGui::GetID("DockSpace");
    if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr) {
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        auto const contentNodeId = buildDockSpace(dockSpaceId);
        ImGui::DockBuilderDockWindow(_documentId.c_str(), contentNodeId);
        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, {}, ImGuiDockNodeFlags_NoWindowMenuButton, &_windowClass);

    if (open) {
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
    renderPanels();

    ImGui::End();
    ImGui::PopID();
}

auto up::shell::Document::buildDockSpace(ImGuiID dockSpaceId) -> ImGuiID {
    return ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_HiddenTabBar);
}
