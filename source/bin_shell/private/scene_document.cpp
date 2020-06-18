// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"
#include "camera_controller.h"
#include "document.h"
#include "scene.h"

#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/draw_imgui.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/renderer.h"

#include <glm/glm.hpp>
#include <SDL.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    class SceneDocument : public Document {
    public:
        explicit SceneDocument(rc<Scene> scene) : Document("SceneDocument"_zsv), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }

        zstring_view displayName() const override { return "Scene"; }

    protected:
        void renderContent(Renderer& renderer) override;
        void renderMenu() override;
        void buildDockSpace(ImGuiID dockId, zstring_view docId) override;

    private:
        void _renderScene(Renderer& renderer, float frameTime);
        void _drawGrid();
        void _resize(Renderer& renderer, glm::ivec2 size);

        rc<Scene> _scene;
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        ArcBallCameraController _cameraController;
        bool _enableGrid = true;
    };

    auto createSceneDocument(rc<Scene> scene) -> box<Document> { return new_box<SceneDocument>(scene); }

    void SceneDocument::renderContent(Renderer& renderer) {
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
            /*auto& self = *static_cast<SceneDocument*>(cmd->UserCallbackData);
            auto& io = ImGui::GetIO();
            self._renderScene(io.DeltaTime);*/
        };

        ImGui::GetWindowDrawList()->AddCallback(callback, this);

        // Note: would prefer to do this in a render callback instead
        //
        _renderScene(renderer, io.DeltaTime);

        auto const pos = ImGui::GetCursorPos();
        ImGui::Image(_bufferView.get(), contentSize);

        ImRect area{pos, pos + contentSize};

        auto const id = ImGui::GetID("SceneControl");
        ImGui::ItemAdd(area, id);
        ImGui::ButtonBehavior(area,
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

    void SceneDocument::renderMenu() {
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

    void SceneDocument::buildDockSpace(ImGuiID dockId, zstring_view docId) {
        auto const centralDockId = ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_None);

        auto contentDockId = centralDockId;
        auto inspectedDockId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Right, 0.25f, nullptr, &contentDockId);
        auto const hierarchyDockId = ImGui::DockBuilderSplitNode(inspectedDockId, ImGuiDir_Down, 0.65f, nullptr, &inspectedDockId);

        ImGui::DockBuilderDockWindow("Inspector", inspectedDockId);
        ImGui::DockBuilderDockWindow("Hierarchy", hierarchyDockId);
        ImGui::DockBuilderDockWindow(docId.c_str(), contentDockId);
    }

    void SceneDocument::_renderScene(Renderer& renderer, float frameTime) {
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

    void SceneDocument::_drawGrid() {
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

    void SceneDocument::_resize(Renderer& renderer, glm::ivec2 size) {
        using namespace up;
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.type = GpuTextureType::Texture2D;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = renderer.device().createTexture2D(desc, {});

        _bufferView = renderer.device().createShaderResourceView(_buffer.get());
    }
} // namespace up::shell
