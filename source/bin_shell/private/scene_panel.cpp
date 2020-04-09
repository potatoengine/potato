// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <glm/glm.hpp>
#include <imgui.h>

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

namespace up::shell {
    class ScenePanel : public shell::Panel {
    public:
        explicit ScenePanel(GpuDevice& device, Scene& scene) : _device(device), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }
        virtual ~ScenePanel() = default;

        void render(Renderer& renderer, float frameTime) override;
        void ui() override;
        bool handleEvent(SDL_Event const& ev) override;
        void tick(float deltaTime) override;

    private:
        void _drawGrid();
        void _resize(glm::ivec2 size);

    private:
        GpuDevice& _device;
        Scene& _scene;
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        ArcBallCameraController _cameraController;
        glm::vec3 _relMotion{0, 0, 0};
        glm::vec3 _relMovement{0, 0, 0};
        bool _enableGrid = true;
        bool _isControllingCamera = false;
    };

    auto createScenePanel(GpuDevice& device, Scene& scene) -> box<Panel> {
        return new_box<ScenePanel>(device, scene);
    }

    void ScenePanel::render(Renderer& renderer, float frameTime) {
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
            _scene.render(ctx);
            renderer.flushDebugDraw(frameTime);
            renderer.endFrame(frameTime);
        }
    }

    void ScenePanel::ui() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu(u8"\uf06e View")) {
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

        if (ImGui::Begin("ScenePanel", nullptr, ImGuiWindowFlags_None)) {
            auto const contentSize = ImGui::GetContentRegionAvail();

            if (contentSize.x <= 0 || contentSize.y <= 0) {
                return;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            if (ImGui::BeginChild("SceneContent", contentSize, false)) {
                glm::vec3 bufferSize = {0, 0, 0};
                if (_buffer != nullptr) {
                    bufferSize = _buffer->dimensions();
                }
                if (bufferSize.x != contentSize.x || bufferSize.y != contentSize.y) {
                    _resize({contentSize.x, contentSize.y});
                }

                auto const pos = ImGui::GetCursorPos();
                ImGui::Image(_bufferView.get(), contentSize);
                ImGui::SetCursorPos(pos);
                ImGui::InvisibleButton("SceneInteract", contentSize);
                if (ImGui::IsItemActive()) {
                    auto& io = ImGui::GetIO();
                    _relMotion.x = io.MouseDelta.x / contentSize.x;
                    _relMotion.y = io.MouseDelta.y / contentSize.y;
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(1);
        }
        ImGui::End();
    }

    bool ScenePanel::handleEvent(SDL_Event const& ev) {
        return false;
    }

    void ScenePanel::tick(float deltaTime) {
        _cameraController.apply(_camera, _relMovement, _relMotion, deltaTime);
        _relMovement = {0, 0, 0};
        _relMotion = {0, 0, 0};
    }

    void ScenePanel::_drawGrid() {
        auto constexpr guidelines = 10;

        // The real intent here is to keep the grid roughly the same spacing in
        // pixels on the screen; this doesn't really accomplish that, though.
        // Improvements welcome.
        //
        auto const cameraPos = _camera.position();
        auto const logDist = std::log2(std::abs(cameraPos.y));
        auto const spacing = std::max(1, static_cast<int>(logDist) - 3);

        int guideSpacing = guidelines * spacing;
        float x = static_cast<float>(static_cast<int>(cameraPos.x / guideSpacing) * guideSpacing);
        float z = static_cast<float>(static_cast<int>(cameraPos.z / guideSpacing) * guideSpacing);

        DebugDrawGrid grid;
        grid.axis2 = {0, 0, 1};
        grid.offset = {x, 0, z};
        grid.halfWidth = 1000;
        grid.spacing = spacing;
        grid.guidelineSpacing = guidelines;
        drawDebugGrid(grid);
    }

    void ScenePanel::_resize(glm::ivec2 size) {
        using namespace up;
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.type = GpuTextureType::Texture2D;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = _device.createTexture2D(desc, {});

        _bufferView = _device.createShaderResourceView(_buffer.get());
    }
} // namespace up
