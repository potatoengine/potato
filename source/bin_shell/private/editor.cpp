// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "editor.h"

#include "potato/format/format.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_writer.h"

#include <imgui.h>
#include <imgui_internal.h>

up::shell::Editor::Editor(zstring_view className) {
    _panelClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _panelClass.DockingAllowUnclassed = false;
    _panelClass.DockingAlwaysTabBar = true;

    _contentClass.ClassId = narrow_cast<ImU32>(reinterpret_cast<uintptr_t>(this));
    _contentClass.DockNodeFlagsOverrideSet =
        ImGuiDockNodeFlags_NoCloseButton | ImGuiDockNodeFlags_NoDockingOverMe | ImGuiDockNodeFlags_NoTabBar;
    _contentClass.DockingAllowUnclassed = false;
    _contentClass.DockingAlwaysTabBar = false;
}

bool up::shell::Editor::updateUi() {
    char editorTitle[128] = {
        0,
    };
    char contentTitle[128] = {
        0,
    };

    format_to(editorTitle, "{}##{}", displayName(), this);
    format_to(contentTitle, "Document##{}", this);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (isClosable()) {
        bool wantOpen = true;
        ImGui::Begin(editorTitle, &wantOpen, ImGuiWindowFlags_NoCollapse);
        _wantClose = _wantClose || !wantOpen;
    }
    else {
        ImGui::Begin(editorTitle, nullptr, ImGuiWindowFlags_NoCollapse);
    }
    ImGui::PopStyleVar(1);

    auto const dockSpaceId = ImGui::GetID("DockSpace");
    if (ImGui::DockBuilderGetNode(dockSpaceId) == nullptr) {
        _dockId = ImGui::DockBuilderAddNode(dockSpaceId, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_NoWindowMenuButton);

        configure();

        ImGui::DockBuilderFinish(dockSpaceId);
    }

    ImGui::DockSpace(dockSpaceId, {}, ImGuiDockNodeFlags_None, &_panelClass);

    ImGui::SetNextWindowClass(&_contentClass);
    ImGui::SetNextWindowDockID(_dockId, ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    auto const contentOpen = ImGui::Begin(contentTitle, nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::PopStyleVar(1);

    if (contentOpen) {
        content();
    }

    ImGui::End();

    for (auto const& panel : _panels) {
        if (panel->open) {
            ImGui::SetNextWindowClass(&_panelClass);
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

    auto const active = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

    ImGui::End();

    return active;
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

    tmp.clear();
    format_append(tmp, "View\\Panels\\{}", panel->title);

    _actions.addAction(
        {.menu = tmp.to_string(),
         .group = "5_panels"_s,
         .checked = [ptr = panel.get()] { return ptr->open; },
         .action =
             [ptr = panel.get()] {
                 ptr->open = !ptr->open;
             }});

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

void up::shell::Editor::activate(bool active, Actions& actions) {
    if (active == _active) {
        return;
    }

    _active = active;

    if (active) {
        actions.addGroup(&_actions);
    }
    else {
        actions.removeGroup(&_actions);
    }
}
