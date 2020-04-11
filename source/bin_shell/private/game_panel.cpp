// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_internal.h>
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
    class GamePanel : public Panel {
    public:
        explicit GamePanel(GpuDevice& device, Scene& scene) : _device(device), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }
        virtual ~GamePanel() = default;

        void render(Renderer& renderer, float frameTime) override;
        void ui() override;
        void tick(float deltaTime) override;

    private:
        void _resize(glm::ivec2 size);

    private:
        GpuDevice& _device;
        Scene& _scene;
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        FlyCameraController _cameraController;
        bool _isInputBound = false;
    };

    auto createGamePanel(GpuDevice& device, Scene& scene) -> box<Panel> {
        return new_box<GamePanel>(device, scene);
    }

    void GamePanel::render(Renderer& renderer, float frameTime) {
        if (_renderCamera == nullptr) {
            _renderCamera = new_box<RenderCamera>();
        }

        if (_buffer != nullptr) {
            renderer.beginFrame();
            auto ctx = renderer.context();

            _renderCamera->resetBackBuffer(_buffer);
            _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
            _scene.render(ctx);
            renderer.flushDebugDraw(frameTime);
            renderer.endFrame(frameTime);
        }
    }

    void GamePanel::ui() {
        auto const contentId = ImGui::GetID("GameContentView");
        auto const* const ctx = ImGui::GetCurrentContext();
        auto const& io = ImGui::GetIO();
        if (_isInputBound) {
            ImGui::SetActiveID(contentId, ctx->CurrentWindow);
            ImGui::SetCaptureRelativeMouseMode(true);

            if (ImGui::IsKeyPressed(SDL_SCANCODE_ESCAPE) && io.KeyShift) {
                _isInputBound = false;
            }
        }
        else {
            if (ctx->ActiveId == contentId) {
                ImGui::ClearActiveID();
            }

            if (ImGui::IsKeyPressed(SDL_SCANCODE_F5, false)) {
                _scene.playing(!_scene.playing());
            }
        }

        if (ImGui::Begin("GamePanel", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
            auto const contentSize = ImGui::GetContentRegionAvail();

            if (contentSize.x <= 0 || contentSize.y <= 0) {
                return;
            }

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
            if (ImGui::BeginChild("GameContent", contentSize, false)) {
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
                ImGui::InvisibleButton("GameContent", contentSize);
                if (ImGui::IsItemActive() && _scene.playing()) {
                    _isInputBound = true;
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(1);
        }
        ImGui::End();
    }

    void GamePanel::tick(float deltaTime) {
        _isInputBound = _isInputBound && _scene.playing();

        if (_isInputBound) {
            glm::vec3 relMotion = {0, 0, 0}, relMove = {0, 0, 0};

            auto& io = ImGui::GetIO();
            relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
            relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
            relMotion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;

            auto keys = SDL_GetKeyboardState(nullptr);
            relMove = {static_cast<float>(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                       static_cast<float>(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_C]),
                       static_cast<float>(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S])};

            _cameraController.apply(_camera, relMove, relMotion, deltaTime);
        }
    }

    void GamePanel::_resize(glm::ivec2 size) {
        using namespace up;
        GpuTextureDesc desc;
        desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
        desc.type = GpuTextureType::Texture2D;
        desc.width = size.x;
        desc.height = size.y;
        _buffer = _device.createTexture2D(desc, {});

        _bufferView = _device.createShaderResourceView(_buffer.get());
    }
} // namespace up::shell
