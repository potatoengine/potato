// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <imgui.h>
#include <SDL.h>

#include "scene.h"

#include "potato/shell/panel.h"
#include "potato/shell/selection.h"

#include "potato/spud/fixed_string_writer.h"

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

    auto createHierarchyPanel(Scene& scene, Selection& selection) -> box<Panel> {
        return new_box<HierarchyPanel>(scene, selection);
    }

    void HierarchyPanel::ui() {
        auto& io = ImGui::GetIO();

        if (!enabled()) {
            return;
        }

        if (ImGui::Begin("Hierarchy", &_enabled, ImGuiWindowFlags_NoCollapse)) {
            for (size_t i = 1, e = _scene.world().archetypes().archetypes(); i != e; ++i) {
                auto const archetypeId = static_cast<ArchetypeId>(i);
                for (Chunk* chunk : _scene.world().getChunks(archetypeId)) {
                    auto const rows = _scene.world().archetypes().layoutOf(archetypeId);
                    for (auto const& row : rows) {
                        if (row.meta == &ComponentMeta::get<Entity>()) {
                            Entity const* const entities = reinterpret_cast<Entity const*>(chunk->data + row.offset);
                            for (int j = 0; j != chunk->header.entities; ++j) {
                                fixed_string_writer label;
                                format_append(label, "Entity (#{})", (unsigned)entities[j].id);
                                ImGui::PushItemWidth(-1.f);
                                if (ImGui::Button(label.c_str())) {
                                    _selection.select(entities[j].id);
                                }
                            }
                        }
                    }
                }
            }
        }
        ImGui::End();
    }
} // namespace up::shell