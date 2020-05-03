// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <glm/glm.hpp>
#include <imgui.h>
#include <SDL.h>

#include "scene.h"
#include "camera.h"
#include "camera_controller.h"
#include "imgui_reflector.h"

#include "potato/shell/panel.h"
#include "potato/shell/selection.h"

#include "potato/ecs/registry.h"

#include "potato/render/gpu_device.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/renderer.h"
#include "potato/render/camera.h"
#include "potato/render/debug_draw.h"
#include "potato/render/context.h"
#include "potato/render/draw_imgui.h"

namespace up::shell {
    class InspectorPanel : public shell::Panel {
    public:
        explicit InspectorPanel(Scene& scene, Selection& selection) : _scene(scene), _selection(selection) {}
        virtual ~InspectorPanel() = default;

        zstring_view displayName() const override { return "Inspector"; }
        void ui() override;

    private:
        Scene& _scene;
        Selection& _selection;
    };

    auto createInspectorPanel(Scene& scene, Selection& selection) -> box<Panel> {
        return new_box<InspectorPanel>(scene, selection);
    }

    void InspectorPanel::ui() {
        auto& io = ImGui::GetIO();

        if (!enabled()) {
            return;
        }

        if (ImGui::Begin("Inspector", &_enabled, ImGuiWindowFlags_NoCollapse)) {
            ComponentId deletedComponent = ComponentId::Unknown;

            _scene.world().interrogateEntityUnsafe(_selection.selected(), [&](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
                if (ImGui::TreeNodeEx(meta->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
                        ImGui::OpenPopup("##component_context_menu");
                    }

                    if (ImGui::BeginPopupContextItem("##component_context_menu")) {
                        if (ImGui::MenuItem(as_char(u8"\uf1f8 Remove"))) {
                            deletedComponent = meta->id;
                        }
                        ImGui::EndPopup();
                    }

                    ImGuiComponentReflector ref;
                    meta->reflect(data, ref);
                    ImGui::TreePop();
                }
            });

            if (deletedComponent != ComponentId::Unknown) {
                _scene.world().removeComponent(_selection.selected(), deletedComponent);
            }

            if (_selection.hasSelection()) {
                if (ImGui::Button(as_char(u8"\uf067 Add Component"))) {
                    ImGui::OpenPopup("##add_component_list");
                }
                if (ImGui::BeginPopup("##add_component_list")) {
                    for (auto const& meta : ComponentRegistry::defaultRegistry().components()) {
                        if (_scene.world().getComponentSlowUnsafe(_selection.selected(), meta.id) == nullptr) {
                            if (ImGui::MenuItem(meta.name.c_str())) {
                                _scene.world().addComponentDefault(_selection.selected(), meta);
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }
        }
        ImGui::End();
    }
} // namespace up::shell
