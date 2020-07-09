// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor.h"

#include "potato/format/format.h"
#include "potato/spud/string_writer.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::Editor::Editor(zstring_view className) {
    _windowClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _windowClass.DockingAllowUnclassed = false;
    _windowClass.DockingAlwaysTabBar = false;
}

void up::shell::Editor::render(Renderer& renderer) {
    if (_title.empty()) {
        string_writer tmp;
        format_append(tmp, "{}##{}", displayName(), this);
        _title = std::move(tmp).to_string();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (isClosable()) {
        bool wantOpen = true;
        ImGui::Begin(_title.c_str(), &wantOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
        _wantClose = !wantOpen;
    }
    else {
        ImGui::Begin(_title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    }
    ImGui::PopStyleVar(1);

    renderMenu();

    if (!_panels.empty() && ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Panels")) {
                for (auto const& panel : _panels) {
                    if (ImGui::MenuItem(panel->title.c_str(), nullptr, panel->open, true)) {
                        panel->open = !panel->open;
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    if (_documentId.empty()) {
        string_writer tmp;
        format_append(tmp, "Document##{}", this);
        _documentId = std::move(tmp).to_string();
    }

    auto const dockSpaceId = ImGui::GetID("DockSpace");
    if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr) {
        ImGui::DockBuilderRemoveNode(dockSpaceId);
        ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace);
        _dockId = ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_HiddenTabBar);

        configure();

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, {}, ImGuiDockNodeFlags_NoWindowMenuButton, &_windowClass);

    ImGui::SetNextWindowClass(&_windowClass);
    ImGui::SetNextWindowDockID(_dockId, ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const contentOpen = ImGui::Begin(_documentId.c_str(), nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar(1);

    if (contentOpen) {
        renderContent(renderer);
    }

    ImGui::End();

    for (auto const& panel : _panels) {
        if (panel->open) {
            ImGui::SetNextWindowClass(&_windowClass);
            ImGui::SetNextWindowDockID(panel->dockId, ImGuiCond_FirstUseEver);
            if (ImGui::Begin(panel->imguiLabel.c_str(), &panel->open, ImGuiWindowFlags_NoCollapse)) {
                panel->update();
            }
            ImGui::End();
        }
    }

    if (_wantClose && !_closed) {
        _closed = handleClose();
    }

    ImGui::End();
}

auto up::shell::Editor::addPanel(string title, PanelUpdate update) -> PanelId {
    UP_ASSERT(!title.empty());
    UP_ASSERT(update != nullptr);

    auto panel = new_box<Panel>();
    panel->title = std::move(title);
    panel->update = std::move(update);

    string_writer tmp;
    format_append(tmp, "{}##{}", panel->title, panel.get());
    panel->imguiLabel = tmp.to_string();

    auto const id = panel->id = ImGui::GetID(panel.get());

    panel->dockId = _dockId;

    _panels.push_back(std::move(panel));
    return id;
}

void up::shell::Editor::dockPanel(PanelId panelId, ImGuiDir dir, PanelId otherId, float size) {
    bool const isOtherContent = otherId == _dockId;

    for (auto const& panel : _panels) {
        if (panel->id == panelId) {
            if (isOtherContent) {
                ImGui::DockBuilderSplitNode(_dockId, dir, size, &panel->dockId, &_dockId);
            }
            else {
                for (auto const& other : _panels) {
                    if (other->id == otherId) {
                        ImGui::DockBuilderSplitNode(other->dockId, dir, size, &panel->dockId, &other->dockId);
                        break;
                    }
                }
            }
            break;
        }
    }
}
