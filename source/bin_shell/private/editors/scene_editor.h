// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "camera.h"
#include "camera_controller.h"
#include "editor.h"
#include "scene.h"
#include "selection.h"

#include "potato/render/camera.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_texture.h"
#include "potato/spud/delegate.h"

#include <glm/glm.hpp>

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
        void configure() override;
        void renderContent(Renderer& renderer) override;
        void renderMenu() override;

    private:
        void _renderScene(Renderer& renderer, float frameTime);
        void _drawGrid();
        void _resize(Renderer& renderer, glm::ivec2 size);
        void _inspector();
        void _hierarchy();

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
        SceneEditor::HandlePlayClicked onPlayClicked) -> box<Editor>;
} // namespace up::shell
