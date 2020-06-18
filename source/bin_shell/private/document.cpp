// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "document.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::Document::Document(zstring_view className) {
    _windowClass.ClassId = ImHashStr(className.data(), className.size());
    _windowClass.DockingAllowUnclassed = false;
    _windowClass.DockingAlwaysTabBar = false;
}

void up::shell::Document::render(Renderer& renderer) {
    ImGui::PushID(this);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const open = ImGui::Begin(displayName().c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    // ImGui::PopStyleVar(1);

    auto const dockId = ImGui::GetID("ContentDockspace");
    ImGui::DockSpace(dockId, {}, ImGuiDockNodeFlags_AutoHideTabBar, &_windowClass);

    if (open) {
        ImGui::SetNextWindowDockID(dockId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_windowClass);
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
        auto const contentOpen = ImGui::Begin("Document", nullptr, ImGuiWindowFlags_NoCollapse);
        // ImGui::PopStyleVar(1);

        if (contentOpen) {
            renderContent(renderer);
        }

        ImGui::End();

        renderMenu();
    }

    ImGui::End();
    ImGui::PopID();
}
