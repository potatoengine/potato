// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "camera.h"

#include "grimm/foundation/box.h"
#include "grimm/foundation/platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/vector.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/stream_util.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/factory.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/texture.h"
#include "grimm/render/renderer.h"
#include "grimm/render/camera.h"
#include "grimm/render/context.h"
#include "grimm/render/node.h"
#include "grimm/render/model.h"
#include "grimm/render/mesh.h"
#include "grimm/render/material.h"
#include "grimm/render/shader.h"
#include "grimm/render/draw_imgui.h"
#include "grimm/render/debug_draw.h"

#include <fmt/chrono.h>
#include <chrono>
#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <imgui.h>

#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

gm::ShellApp::ShellApp() = default;

gm::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _renderer.reset();
    _root.reset();
    _camera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int gm::ShellApp::initialize() {
    using namespace gm;

    _window = SDL_CreateWindow("Grimm Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create window", nullptr);
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_window.get(), &wmInfo)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not get window info", _window.get());
    }

#if GM_GPU_ENABLE_D3D11
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = gpu::CreateFactoryNull();
        _device = factory->createDevice(0);
    }

    if (_device == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not find device", _window.get());
        return 1;
    }

    _renderer = make_box<Renderer>(_fileSystem, _device);

#if GM_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Failed to create swap chain", _window.get());
        return 1;
    }

    _camera = make_box<RenderCamera>(_swapChain);

    auto material = _renderer->loadMaterialSync("resources/materials/basic.json");
    auto mesh = _renderer->loadMeshSync("resources/meshes/cube.model");
    auto model = make_box<Model>(std::move(mesh), std::move(material));
    _root = make_box<Node>(std::move(model));
    _root->transform(translate(glm::identity<glm::mat4x4>(), {0, 5, 0}));

    auto imguiVertShader = _renderer->loadShaderSync("resources/shaders/imgui.vs_5_0.cbo");
    auto imguiPixelShader = _renderer->loadShaderSync("resources/shaders/imgui.ps_5_0.cbo");

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    _drawImgui.createResources(*_device);

    return 0;
}

namespace {
    class CameraController {
    public:
        virtual ~CameraController() = default;
        virtual void apply(gm::Camera& camera, glm::vec3 relativeMovement, glm::vec2 relativeMotion, float frameTime) = 0;
    };

    class FlyCameraController : public CameraController {
    public:
        void apply(gm::Camera& camera, glm::vec3 relativeMovement, glm::vec2 relativeMotion, float frameTime) override {
            glm::vec3 movement =
                camera.right() * relativeMovement.x +
                camera.up() * relativeMovement.y +
                camera.view() * relativeMovement.z;

            glm::vec3 pos = camera.position() + movement * _moveSpeedPerSec * frameTime;

            _yaw -= relativeMotion.x;
            _pitch -= relativeMotion.y;

            glm::vec3 view{0, 0, -1};
            view = glm::rotate(view, _pitch, {1, 0, 0});
            view = glm::rotate(view, _yaw, {0, 1, 0});

            camera.lookAt(pos, pos + view, {0, 1, 0});
        }

    private:
        float _moveSpeedPerSec = 10;
        float _rotateRadiansPerSec = 1;
        float _yaw = 0;
        float _pitch = 0;
    };

    class ArcBallCameraController : public CameraController {
    public:
        void apply(gm::Camera& camera, glm::vec3 relativeMovement, glm::vec2 relativeMotion, float frameTime) override {
            _target += relativeMovement * 10.f * frameTime;

            _yaw += relativeMotion.x;
            _pitch -= relativeMotion.y;

            glm::vec3 pos{0, 0, _boomLength};
            pos = glm::rotate(pos, _pitch, {1, 0, 0});
            pos = glm::rotate(pos, _yaw, {0, 1, 0});

            camera.lookAt(pos + _target, _target, {0, 1, 0});
        }

    private:
        glm::vec3 _target = {0, 5, 0};
        float _boomLength = 10;
        float _yaw = 0;
        float _pitch = -45;
    };
} // namespace

void gm::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    std::chrono::high_resolution_clock clock;

    auto now = clock.now();
    auto duration = now - now;
    float frameTime = 0;

    float camSpeed = 10;
    float camRotSpeed = 800;

    Camera camera;
    camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});

    box<CameraController> controller = make_box<FlyCameraController>();
    glm::vec3 arcCenter = {0, 0, 0};

    float objRotateInput = 0;

    while (isRunning()) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                switch (ev.window.event) {
                case SDL_WINDOWEVENT_CLOSE:
                    onWindowClosed();
                    break;
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    onWindowSizeChanged();
                    break;
                }
                break;
            case SDL_KEYDOWN:
                if (ev.key.keysym.scancode == SDL_SCANCODE_F) {
                    controller = make_box<FlyCameraController>();
                }
                if (ev.key.keysym.scancode == SDL_SCANCODE_B) {
                    controller = make_box<ArcBallCameraController>();
                }
                break;
            }
            _drawImgui.handleEvent(ev);
        }

        glm::vec3 relativeMovement = {0, 0, 0};
        glm::vec2 relativeMotion = {0, 0};

        if (!imguiIO.WantCaptureKeyboard) {
            auto keys = SDL_GetKeyboardState(nullptr);
            relativeMovement = {static_cast<float>(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                                static_cast<float>(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LCTRL]),
                                static_cast<float>(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S])};
        }

        int relx, rely;
        int buttons = SDL_GetRelativeMouseState(&relx, &rely);
        bool isMouseMove = buttons != 0 && !imguiIO.WantCaptureMouse;
        SDL_SetRelativeMouseMode(isMouseMove ? SDL_TRUE : SDL_FALSE);
        if (isMouseMove) {
            relativeMotion.x = static_cast<float>(relx) / 800;
            relativeMotion.y = static_cast<float>(rely) / 600;
        }

        controller->apply(camera, relativeMovement, relativeMotion, frameTime);

        const float radiansPerSec = 2;
        const float rotateRads = radiansPerSec * frameTime;
        objRotateInput += frameTime;
        _root->transform(glm::rotate(glm::rotate(glm::translate(glm::identity<glm::mat4x4>(), {0, 5, 0}), objRotateInput, {0, 1, 0}), std::sin(objRotateInput), {1, 0, 0}));

        gpu::Viewport viewport;
        int width, height;
        SDL_GetWindowSize(_window.get(), &width, &height);
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);

        imguiIO.DisplaySize.x = viewport.width;
        imguiIO.DisplaySize.y = viewport.height;
        _drawImgui.beginFrame();
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Grimm")) {
                if (ImGui::MenuItem("Quit")) {
                    return;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (ImGui::Begin("Camera")) {
            auto pos = camera.position();
            auto view = camera.view();
            auto right = camera.right();
            auto up = camera.up();
            ImGui::InputFloat3("Position", &pos.x);
            ImGui::InputFloat3("View", &view.x);
            ImGui::InputFloat3("Right", &right.x);
            ImGui::InputFloat3("Up", &up.x);
            camera.lookAt(pos, pos + view, up);
        }
        ImGui::End();

        if (ImGui::Begin("Statistics")) {
            auto micro = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();

            fixed_string_writer<128> buffer;
            format_into(buffer, "{}us", micro);
            ImGui::LabelText("Frametime", "%s", buffer.c_str());
            buffer.clear();
            format_into(buffer, "{}", 1 / frameTime);
            ImGui::LabelText("FPS", "%s", buffer.c_str());
        }
        ImGui::End();

        for (int i = -10; i <= 10; ++i) {
            drawDebugLine({-10, 0, i}, {10, 0, i}, i == 0 ? glm::vec4{1, 0, 0, 1} : glm::vec4{0.3f, 0.3f, 0.3f, 1.f});
            drawDebugLine({i, 0, -10}, {i, 0, 10}, i == 0 ? glm ::vec4{0, 0, 1, 1} : glm::vec4{0.3f, 0.3f, 0.3f, 1.f});
        }
        drawDebugLine({0, -10, 0}, {0, +10, 0}, {0, 1, 0, 1});

        _renderer->beginFrame();
        auto ctx = _renderer->context();
        _camera->beginFrame(ctx, camera.matrix());
        _root->render(ctx);

        _drawImgui.endFrame(*_device, _renderer->commandList());

        _camera->endFrame(ctx);
        _renderer->endFrame(frameTime);
        _swapChain->present();

        auto endFrame = clock.now();
        duration = endFrame - now;
        frameTime = static_cast<float>(duration.count() / 1000000000.0);
        now = endFrame;
    }
}

void gm::ShellApp::quit() {
    _running = false;
}

void gm::ShellApp::onWindowClosed() {
    quit();
}

void gm::ShellApp::onWindowSizeChanged() {
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _camera->resetSwapChain(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _camera->resetSwapChain(_swapChain);
}
