// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <glm/glm.hpp>
#include <imgui.h>
#include <SDL.h>

#include "scene.h"
#include "camera.h"
#include "camera_controller.h"

#include "potato/shell/panel.h"
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
        explicit InspectorPanel(Scene& scene) : _scene(scene) {}
        virtual ~InspectorPanel() = default;

        void ui() override;

    private:
        Scene& _scene;
    };

    auto createInspectorPanel(Scene& scene) -> box<Panel> {
        return new_box<InspectorPanel>(scene);
    }

    namespace {
        class ImGuiComponentReflector final : public up::ComponentReflector {
        protected:
            void onField(up::zstring_view name) override {
                _name = name;
            }

            void onValue(int& value) override {
                ImGui::InputInt(_name.c_str(), &value);
            }

            void onValue(float& value) override {
                ImGui::InputFloat(_name.c_str(), &value);
            }

            void onValue(up::EntityId value) override {
                ImGui::LabelText(_name.c_str(), "%u", (unsigned)value);
            }

            void onValue(glm::vec3& value) override {
                ImGui::InputFloat3(_name.c_str(), &value.x);
            }

        private:
            up::zstring_view _name;
        };
    } // namespace

    void InspectorPanel::ui() {
        auto& io = ImGui::GetIO();

        if (ImGui::Begin(u8"\uf085 Inspector")) {
            _scene.world().interrogateEntity(_scene.main(), [](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
                if (ImGui::CollapsingHeader(meta->name.c_str())) {
                    ImGuiComponentReflector ref;
                    meta->reflect(data, ref);
                }
            });
        }
        ImGui::End();
    }
} // namespace up::shell
