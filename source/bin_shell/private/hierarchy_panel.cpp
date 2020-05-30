// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "panel.h"
#include "scene.h"
#include "selection.h"

#include "potato/spud/fixed_string_writer.h"

#include <SDL.h>
#include <imgui.h>

namespace up::shell {
    class HierarchyPanel : public shell::Panel {
    public:
        explicit HierarchyPanel(Scene& scene, Selection& selection) : _scene(scene), _selection(selection) {}
        virtual ~HierarchyPanel() = default;

        zstring_view displayName() const override { return "Hierarchy"; }
        void ui() override;

    private:
        Scene& _scene;
        Selection& _selection;
    };

    auto createHierarchyPanel(Scene& scene, Selection& selection) -> box<Panel> { return new_box<HierarchyPanel>(scene, selection); }

    void HierarchyPanel::ui() {
        if (!enabled()) {
            return;
        }

        if (ImGui::Begin("Hierarchy", &_enabled, ImGuiWindowFlags_NoCollapse)) {
            fixed_string_writer label;

            for (auto const& chunk : _scene.world().chunks()) {
                for (EntityId entityId : chunk->entities()) {
                    label.clear();
                    format_append(label, "Entity (#{})", entityId);

                    bool selected = entityId == _selection.selected();
                    if (ImGui::Selectable(label.c_str(), selected)) {
                        _selection.select(entityId);
                    }
                    if (selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
        }
        ImGui::End();
    }
} // namespace up::shell
