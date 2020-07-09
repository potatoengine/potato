// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"

#include "potato/render/camera.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"

namespace up::shell {
    class GameEditor : public Editor {
    public:
        explicit GameEditor(rc<Scene> scene)
            : Editor("GameEditor"_zsv)
            , _scene(std::move(scene))
            , _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }

        zstring_view displayName() const override { return "Game"; }

    protected:
        void configure() override {}
        void renderContent(Renderer& renderer) override;
        void renderMenu() override;
        bool hasMenu() override { return true; }

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

    auto createGameEditor(rc<Scene> scene) -> box<Editor>;
} // namespace up::shell
