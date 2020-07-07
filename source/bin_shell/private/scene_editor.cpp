// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "imgui_reflector.h"
#include "scene.h"
#include "selection.h"

#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/draw_imgui.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/renderer.h"
#include "potato/spud/delegate.h"
#include "potato/spud/fixed_string_writer.h"

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    class SceneEditor : public Editor {
    public:
        using EnumerateComponents = delegate<view<ComponentMeta>()>;
        using HandlePlayClicked = delegate<void(rc<Scene>)>;

        explicit SceneEditor(rc<Scene> scene, EnumerateComponents components, HandlePlayClicked onPlayClicked)
            : Editor("SceneEditor"_zsv)
            , _scene(std::move(scene))
            , _cameraController(_camera)
            , _components(std::move(components))
            , _onPlayClicked(std::move(onPlayClicked)) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
            _selection.select(_scene->root());
        }

        zstring_view displayName() const override { return "Scene"; }

        void tick(float deltaTime) override;

    protected:
        void renderContent(Renderer& renderer) override;
        void renderMenu() override;
        void renderPanels() override;
        auto buildDockSpace(ImGuiID dockId) -> ImGuiID override;

    private:
        void _renderScene(Renderer& renderer, float frameTime);
        void _drawGrid();
        void _resize(Renderer& renderer, glm::ivec2 size);
        void _renderInspector();
        void _renderHierarchy();

        rc<Scene> _scene;
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        ArcBallCameraController _cameraController;
        Selection _selection;
        EnumerateComponents _components;
        HandlePlayClicked _onPlayClicked;
        bool _enableGrid = true;
    };

    auto createSceneEditor(
        rc<Scene> scene,
        SceneEditor::EnumerateComponents components,
        SceneEditor::HandlePlayClicked onPlayClicked) -> box<Editor> {
        return new_box<SceneEditor>(std::move(scene), std::move(components), std::move(onPlayClicked));
    }

    void SceneEditor::tick(float deltaTime) {
        _scene->tick(deltaTime);
        _scene->flush();
    }

    void SceneEditor::renderContent(Renderer& renderer) {
        auto& io = ImGui::GetIO();

        auto const contentSize = ImGui::GetContentRegionAvail();

        if (contentSize.x <= 0 || contentSize.y <= 0) {
            return;
        }

        ImGui::BeginChild("SceneContent", contentSize, false);

        glm::vec3 bufferSize = {0, 0, 0};
        if (_buffer != nullptr) {
            bufferSize = _buffer->dimensions();
        }
        if (bufferSize.x != contentSize.x || bufferSize.y != contentSize.y) {
            _resize(renderer, {contentSize.x, contentSize.y});
        }

        glm::vec3 movement = {0, 0, 0};
        glm::vec3 motion = {0, 0, 0};

        auto callback = [](const ImDrawList* list, const ImDrawCmd* cmd) {
            // Note: we'd like to do this here, but we'll need our own render data since we're in
            // the middle of using the Renderer to draw the ImGui data at the time this is called.
            /*auto& self = *static_cast<SceneEditor*>(cmd->UserCallbackData);
            auto& io = ImGui::GetIO();
            self._renderScene(io.DeltaTime);*/
        };

        ImGui::GetWindowDrawList()->AddCallback(callback, this);

        // Note: would prefer to do this in a render callback instead
        //
        _renderScene(renderer, io.DeltaTime);

        auto const pos = ImGui::GetCursorScreenPos();
        ImGui::Image(_bufferView.get(), contentSize);

        ImRect area{pos, pos + contentSize};

        auto const id = ImGui::GetID("SceneControl");
        ImGui::ItemAdd(area, id);
        ImGui::ButtonBehavior(
            area,
            id,
            nullptr,
            nullptr,
            ImGuiButtonFlags_PressedOnClick | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle);
        if (ImGui::IsItemActive()) {
            ImGui::CaptureMouseFromApp();

            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                motion.x = io.MouseDelta.x / contentSize.x * 2;
                motion.y = io.MouseDelta.y / contentSize.y * 2;
            }
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                movement.x = io.MouseDelta.x;
                movement.y = io.MouseDelta.y;
            }
        }

        if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
            motion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;
        }

        _cameraController.apply(_camera, movement, motion, io.DeltaTime);

        ImGui::EndChild();
    }

    void SceneEditor::renderMenu() {
        if (_onPlayClicked != nullptr) {
            if (ImGui::BeginMenuBar()) {
                if (ImGui::MenuItem("Play")) {
                    _onPlayClicked(_scene);
                }
                ImGui::EndMenuBar();
            }
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu(as_char(u8"\uf06e View"))) {
                if (ImGui::BeginMenu("Options")) {
                    if (ImGui::MenuItem("Grid")) {
                        _enableGrid = !_enableGrid;
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    auto SceneEditor::buildDockSpace(ImGuiID dockId) -> ImGuiID {
        auto contentNodeId = ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_HiddenTabBar);
        auto inspectedNodeId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Right, 0.25f, nullptr, &contentNodeId);
        auto const hierarchyNodeId =
            ImGui::DockBuilderSplitNode(inspectedNodeId, ImGuiDir_Down, 0.65f, nullptr, &inspectedNodeId);

        ImGui::DockBuilderDockWindow("Inspector##SceneInspector", inspectedNodeId);
        ImGui::DockBuilderDockWindow("Hierarchy##SceneEditor", hierarchyNodeId);

        return contentNodeId;
    }

    void SceneEditor::_renderScene(Renderer& renderer, float frameTime) {
        if (_renderCamera == nullptr) {
            _renderCamera = new_box<RenderCamera>();
        }

        if (_buffer != nullptr) {
            renderer.beginFrame();
            auto ctx = renderer.context();

            _renderCamera->resetBackBuffer(_buffer);
            if (_enableGrid) {
                _drawGrid();
            }
            _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
            if (_scene != nullptr) {
                _scene->render(ctx);
            }
            renderer.flushDebugDraw(frameTime);
            renderer.endFrame(frameTime);
        }
    }

    void SceneEditor::_drawGrid() {
        auto constexpr guidelines = 10;

        // The real intent here is to keep the grid roughly the same spacing in
        // pixels on the screen; this doesn't really accomplish that, though.
        // Improvements welcome.
        //
        auto const cameraPos = _camera.position();
        auto const logDist = std::log2(std::abs(cameraPos.y));
        auto const spacing = std::max(1, static_cast<int>(logDist) - 3);

        auto const guideSpacing = static_cast<float>(guidelines * spacing);
        float x = std::trunc(cameraPos.x / guideSpacing) * guideSpacing;
        float z = std::trunc(cameraPos.z / guideSpacing) * guideSpacing;

        DebugDrawGrid grid;
        grid.axis2 = {0, 0, 1};
        grid.offset = {x, 0, z};
        grid.halfWidth = 1000;
        grid.spacing = spacing;
        grid.guidelineSpacing = guidelines;
        drawDebugGrid(grid);
    }

    void SceneEditor::_resize(Renderer& renderer, glm::ivec2 size) {
        using namespace up;
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.type = GpuTextureType::Texture2D;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = renderer.device().createTexture2D(desc, {});

        _bufferView = renderer.device().createShaderResourceView(_buffer.get());
    }

    void SceneEditor::renderPanels() {
        _renderInspector();
        _renderHierarchy();
    }

    void SceneEditor::_renderInspector() {
        ImGui::SetNextWindowClass(&documentClass());
        ImGui::Begin("Inspector##SceneInspector", nullptr, ImGuiWindowFlags_NoCollapse);
        ComponentId deletedComponent = ComponentId::Unknown;

        if (_scene != nullptr) {
            _scene->world().interrogateEntityUnsafe(
                _selection.selected(),
                [&](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
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
                        meta->ops.serialize(data, ref);
                        ImGui::TreePop();
                    }
                });

            if (deletedComponent != ComponentId::Unknown) {
                _scene->world().removeComponent(_selection.selected(), deletedComponent);
            }

            if (_selection.hasSelection()) {
                if (ImGui::Button(as_char(u8"\uf067 Add Component"))) {
                    ImGui::OpenPopup("##add_component_list");
                }
                if (ImGui::BeginPopup("##add_component_list")) {
                    for (auto const& meta : _components()) {
                        if (_scene->world().getComponentSlowUnsafe(_selection.selected(), meta.id) == nullptr) {
                            if (ImGui::MenuItem(meta.name.c_str())) {
                                _scene->world().addComponentDefault(_selection.selected(), meta);
                            }
                        }
                    }
                    ImGui::EndPopup();
                }
            }
        }

        ImGui::End();
    }

    void SceneEditor::_renderHierarchy() {
        ImGui::SetNextWindowClass(&documentClass());
        ImGui::Begin("Hierarchy##SceneEditor", nullptr, ImGuiWindowFlags_NoCollapse);
        constexpr int label_length = 64;

        fixed_string_writer<label_length> label;

        if (_scene != nullptr) {
            for (auto const& chunk : _scene->world().chunks()) {
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
