// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"

#include "potato/foundation/box.h"
#include "potato/foundation/platform.h"
#include "potato/foundation/unique_resource.h"
#include "potato/foundation/vector.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/stream_util.h"
#include "potato/gpu/device.h"
#include "potato/gpu/factory.h"
#include "potato/gpu/command_list.h"
#include "potato/gpu/swap_chain.h"
#include "potato/gpu/texture.h"
#include "potato/render/renderer.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/node.h"
#include "potato/render/model.h"
#include "potato/render/mesh.h"
#include "potato/render/material.h"
#include "potato/render/shader.h"
#include "potato/render/draw_imgui.h"
#include "potato/render/debug_draw.h"

#include <chrono>
#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <imgui.h>

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

up::ShellApp::ShellApp() : _logger("shell") {}

up::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _renderer.reset();
    _root.reset();
    _camera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::ShellApp::initialize() {
    using namespace up;

    _window = SDL_CreateWindow("Potato Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", "Could not create window", nullptr);
    }

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (!SDL_GetWindowWMInfo(_window.get(), &wmInfo)) {
        _errorDialog("Could not get window info");
        return 1;
    }

#if UP_GPU_ENABLE_D3D11
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
        _errorDialog("Could not find device");
        return 1;
    }

    _renderer = new_box<Renderer>(_fileSystem, _device);

#if UP_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        _errorDialog("Failed to create swap chain");
        return 1;
    }

    _camera = new_box<RenderCamera>(_swapChain);

    auto material = _renderer->loadMaterialSync("resources/materials/basic.json");
    if (material == nullptr) {
        _errorDialog("Failed to load basic material");
        return 1;
    }

    auto mesh = _renderer->loadMeshSync("resources/meshes/cube.model");
    if (mesh == nullptr) {
        _errorDialog("Failed to load cube mesh");
        return 1;
    }

    auto model = new_box<Model>(std::move(mesh), std::move(material));
    _root = new_box<Node>(std::move(model));
    _root->transform(translate(glm::identity<glm::mat4x4>(), {0, 5, 0}));

    auto imguiVertShader = _renderer->loadShaderSync("resources/shaders/imgui.vs_5_0.cbo");
    auto imguiPixelShader = _renderer->loadShaderSync("resources/shaders/imgui.ps_5_0.cbo");
    if (imguiVertShader == nullptr || imguiPixelShader == nullptr) {
        _errorDialog("Failed to load imgui shaders");
        return 1;
    }

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    _drawImgui.createResources(*_device);

    return 0;
}

void up::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    std::chrono::high_resolution_clock clock;

    auto now = clock.now();
    auto duration = now - now;
    float frameTime = 0;

    float camSpeed = 10;
    float camRotSpeed = 800;

    Camera camera;
    camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});

    box<CameraController> controller = new_box<ArcBallCameraController>();
    glm::vec3 arcCenter = {0, 0, 0};

    float objRotateInput = 0;

    while (isRunning()) {
        int wheelAction = 0;

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
                    controller = new_box<FlyCameraController>();
                }
                if (ev.key.keysym.scancode == SDL_SCANCODE_B) {
                    controller = new_box<ArcBallCameraController>();
                }
                break;
            case SDL_MOUSEWHEEL:
                wheelAction += (ev.wheel.y > 0 ? 1 : ev.wheel.y < 0 ? -1 : 0) * (ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1 : 1);
                break;
            }
            _drawImgui.handleEvent(ev);
        }

        glm::vec3 relativeMovement = {0, 0, 0};
        glm::vec3 relativeMotion = {0, 0, 0};

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
        relativeMotion.z = static_cast<float>(wheelAction);

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
            if (ImGui::BeginMenu("Potato")) {
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

void up::ShellApp::quit() {
    _running = false;
}

void up::ShellApp::onWindowClosed() {
    quit();
}

void up::ShellApp::onWindowSizeChanged() {
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _camera->resetSwapChain(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _camera->resetSwapChain(_swapChain);
}

void up::ShellApp::_errorDialog(zstring_view message) {
    _logger.error("Fatal error: {}", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), _window.get());
}
