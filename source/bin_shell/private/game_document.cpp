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
    class GameDocument : public Document {
    public:
        explicit GameDocument(rc<Scene> scene) : Document("GameDocument"_zsv), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }

        zstring_view displayName() const override { return "Game"; }

    protected:
        virtual void renderContent(Renderer& renderer) override;

    private:
        void _renderScene(Renderer& renderer, float frameTime);
        void _resize(Renderer& renderer, glm::ivec2 size);

        rc<Scene> _scene;
        rc<GpuTexture> _buffer;
        box<GpuResourceView> _bufferView;
        box<RenderCamera> _renderCamera;
        Camera _camera;
        FlyCameraController _cameraController;
        bool _isInputBound = false;
    };

    auto createGameDocument(rc<Scene> scene) -> box<Document> { return new_box<GameDocument>(scene); }

    void GameDocument::renderContent(Renderer& renderer) {
        auto const contentId = ImGui::GetID("GameContentView");
        auto const* const ctx = ImGui::GetCurrentContext();
        auto const& io = ImGui::GetIO();

        if (ImGui::IsKeyPressed(SDL_SCANCODE_F5, false)) {
            _scene->playing(!_scene->playing());
        }

        if (ImGui::IsKeyPressed(SDL_SCANCODE_ESCAPE) && io.KeyShift) {
            _isInputBound = false;
        }

        _isInputBound = _isInputBound && _scene->playing();

        if (_isInputBound) {
            ImGui::SetActiveID(contentId, ctx->CurrentWindow);
            ImGui::SetCaptureRelativeMouseMode(true);

            glm::vec3 relMotion = {0, 0, 0};
            glm::vec3 relMove = {0, 0, 0};

            relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
            relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
            relMotion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;

            relMove = {static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_D)) - static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_A)),
                static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_SPACE)) - static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_C)),
                static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_W)) - static_cast<int>(ImGui::IsKeyPressed(SDL_SCANCODE_S))};

            _cameraController.apply(_camera, relMove, relMotion, io.DeltaTime);
        }
        else {
            if (ctx->ActiveId == contentId) {
                ImGui::ClearActiveID();
            }
        }

        auto const contentSize = ImGui::GetContentRegionAvail();

        if (contentSize.x <= 0 || contentSize.y <= 0) {
            return;
        }

        if (ImGui::BeginChild("GameContent", contentSize, false)) {
            glm::vec3 bufferSize = {0, 0, 0};
            if (_buffer != nullptr) {
                bufferSize = _buffer->dimensions();
            }
            if (bufferSize.x != contentSize.x || bufferSize.y != contentSize.y) {
                _resize(renderer, {contentSize.x, contentSize.y});
            }

            _renderScene(renderer, io.DeltaTime);

            auto const pos = ImGui::GetCursorPos();
            ImGui::Image(_bufferView.get(), contentSize);
            ImGui::SetCursorPos(pos);
            ImGui::InvisibleButton("GameContent", contentSize);
            if (ImGui::IsItemActive() && _scene != nullptr && _scene->playing()) {
                _isInputBound = true;
            }
        }
        ImGui::EndChild();
    }

    void GameDocument::_renderScene(Renderer& renderer, float frameTime) {
        if (_renderCamera == nullptr) {
            _renderCamera = new_box<RenderCamera>();
        }

        if (_buffer != nullptr) {
            renderer.beginFrame();
            auto ctx = renderer.context();

            _renderCamera->resetBackBuffer(_buffer);
            _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
            if (_scene != nullptr) {
                _scene->render(ctx);
            }
            renderer.flushDebugDraw(frameTime);
            renderer.endFrame(frameTime);
        }
    }

    void GameDocument::_resize(Renderer& renderer, glm::ivec2 size) {
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
