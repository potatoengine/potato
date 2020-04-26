// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <glm/glm.hpp>
#include <imgui.h>
#include <SDL.h>

#include "scene.h"
#include "camera.h"
#include "camera_controller.h"

#include "potato/shell/panel.h"
#include "potato/shell/selection.h"

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

            void onValue(glm::quat& value) override {
                auto euler = glm::eulerAngles(value);
                auto eulerDegrees = glm::vec3(
                    glm::degrees(euler.x),
                    glm::degrees(euler.y),
                    glm::degrees(euler.z));

                if (ImGui::SliderFloat3(_name.c_str(), &eulerDegrees.x, 0, +359.f)) {
                    value = glm::vec3(
                        glm::radians(eulerDegrees.x),
                        glm::radians(eulerDegrees.y),
                        glm::radians(eulerDegrees.z));
                }
            }

        private:
            up::zstring_view _name;
        };
    } // namespace

    void InspectorPanel::ui() {
        auto& io = ImGui::GetIO();

        if (!enabled()) {
            return;
        }

        if (ImGui::Begin(as_char(u8"\uf085 Inspector"), &_enabled, ImGuiWindowFlags_NoCollapse)) {
            _scene.world().interrogateEntityUnsafe(_selection.selected(), [](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
                if (ImGui::TreeNodeEx(meta->name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGuiComponentReflector ref;
                    meta->reflect(data, ref);
                    ImGui::TreePop();
                }
            });
        }
        ImGui::End();
    }
} // namespace up::shell
