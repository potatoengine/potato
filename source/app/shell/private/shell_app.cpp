// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "scene.h"

#include "potato/foundation/box.h"
#include "potato/foundation/platform.h"
#include "potato/foundation/unique_resource.h"
#include "potato/foundation/vector.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/path.h"
#include "potato/filesystem/json.h"
#include "potato/filesystem/native.h"
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
#include "potato/ecs/world.h"
#include "potato/ecs/query.h"

#include <chrono>
#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <imgui.h>

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <nlohmann/json.hpp>

struct up::ShellApp::InputState {
    glm::vec3 relativeMovement = {0, 0, 0};
    glm::vec3 relativeMotion = {0, 0, 0};
};

up::ShellApp::ShellApp() : _scene(new_box<Scene>()), _logger("shell"), _inputState(new_box<InputState>()) {}

up::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _renderer.reset();
    _renderCamera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::ShellApp::initialize() {
    using namespace up;

    zstring_view configPath = "shell.config.json";
    if (_fileSystem.fileExists(configPath)) {
        _loadConfig(configPath);
    }

    if (!_resourceDir.empty()) {
        _fileSystem.currentWorkingDirectory(_resourceDir.c_str());
    }

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
        auto factory = CreateFactoryD3D11();
        _device = factory->createDevice(0);
    }
#endif
    if (_device == nullptr) {
        auto factory = CreateFactoryNull();
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

    _renderCamera = new_box<RenderCamera>(_swapChain);

    auto material = _renderer->loadMaterialSync("resources/materials/full.json");
    if (material == nullptr) {
        _errorDialog("Failed to load basic material");
        return 1;
    }

    auto mesh = _renderer->loadMeshSync("resources/meshes/cube.model");
    if (mesh == nullptr) {
        _errorDialog("Failed to load cube mesh");
        return 1;
    }

    _scene->create(new_shared<Model>(std::move(mesh), std::move(material)));

    auto imguiVertShader = _renderer->loadShaderSync("resources/shaders/imgui.vs_5_0.cbo");
    auto imguiPixelShader = _renderer->loadShaderSync("resources/shaders/imgui.ps_5_0.cbo");
    if (imguiVertShader == nullptr || imguiPixelShader == nullptr) {
        _errorDialog("Failed to load imgui shaders");
        return 1;
    }

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    _drawImgui.createResources(*_device);

    _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
    _cameraController = new_box<ArcBallCameraController>(_camera);

    return 0;
}

void up::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    std::chrono::high_resolution_clock clock;

    auto now = clock.now();
    _lastFrameDuration = now - now;

    while (isRunning()) {
        _processEvents();
        _tick();
        _render();

        auto endFrame = clock.now();
        _lastFrameDuration = endFrame - now;
        _lastFrameTime = static_cast<float>(_lastFrameDuration.count() / 1000000000.0);
        now = endFrame;
    }
}

void up::ShellApp::quit() {
    _running = false;
}

void up::ShellApp::_onWindowClosed() {
    quit();
}

void up::ShellApp::_onWindowSizeChanged() {
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _renderCamera->resetSwapChain(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _renderCamera->resetSwapChain(_swapChain);
}

void up::ShellApp::_processEvents() {
    auto& imguiIO = ImGui::GetIO();

    _inputState->relativeMotion = {0, 0, 0};
    _inputState->relativeMovement = {0, 0, 0};

    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        switch (ev.type) {
        case SDL_QUIT:
            return;
        case SDL_WINDOWEVENT:
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                _onWindowClosed();
                break;
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                _onWindowSizeChanged();
                break;
            }
            break;
        case SDL_KEYDOWN:
            if (ev.key.keysym.scancode == SDL_SCANCODE_F) {
                _cameraController = new_box<FlyCameraController>(_camera);
            }
            if (ev.key.keysym.scancode == SDL_SCANCODE_B) {
                _cameraController = new_box<ArcBallCameraController>(_camera);
            }
            if (ev.key.keysym.scancode == SDL_SCANCODE_F5) {
                _paused = !_paused;
            }
            break;
        case SDL_MOUSEWHEEL:
            _inputState->relativeMotion.z += (ev.wheel.y > 0.f ? 1.f : ev.wheel.y < 0 ? -1.f : 0.f) * (ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.f : 1.f);
            break;
        }
        _drawImgui.handleEvent(ev);
    }

    if (!imguiIO.WantCaptureKeyboard) {
        auto keys = SDL_GetKeyboardState(nullptr);
        _inputState->relativeMovement = {static_cast<float>(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                            static_cast<float>(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_LCTRL]),
                            static_cast<float>(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S])};
    }

    int relx, rely;
    int buttons = SDL_GetRelativeMouseState(&relx, &rely);
    bool isMouseMove = buttons != 0 && !imguiIO.WantCaptureMouse;
    SDL_SetRelativeMouseMode(isMouseMove ? SDL_TRUE : SDL_FALSE);
    if (isMouseMove) {
        _inputState->relativeMotion.x = static_cast<float>(relx) / 800;
        _inputState->relativeMotion.y = static_cast<float>(rely) / 600;
    }
}

void up::ShellApp::_tick() {
    _cameraController->apply(_camera, _inputState->relativeMovement, _inputState->relativeMotion, _lastFrameTime);

    if (!_paused) {
        _scene->tick(_lastFrameTime);
    }
}

void up::ShellApp::_render() {
    GpuViewportDesc viewport;
    int width, height;
    SDL_GetWindowSize(_window.get(), &width, &height);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);

    auto& imguiIO = ImGui::GetIO();
    imguiIO.DisplaySize.x = viewport.width;
    imguiIO.DisplaySize.y = viewport.height;

    _drawUI();

    for (int i = -10; i <= 10; ++i) {
        drawDebugLine({-10, 0, i}, {10, 0, i}, i == 0 ? glm::vec4{1, 0, 0, 1} : glm::vec4{0.3f, 0.3f, 0.3f, 1.f});
        drawDebugLine({i, 0, -10}, {i, 0, 10}, i == 0 ? glm ::vec4{0, 0, 1, 1} : glm::vec4{0.3f, 0.3f, 0.3f, 1.f});
    }
    drawDebugLine({0, -10, 0}, {0, +10, 0}, {0, 1, 0, 1});

    _renderer->beginFrame();
    auto ctx = _renderer->context();
    _renderCamera->beginFrame(ctx, _camera.matrix());
    _scene->render(ctx);

    _drawImgui.endFrame(*_device, _renderer->commandList());

    _renderCamera->endFrame(ctx);
    _renderer->endFrame(_lastFrameTime);
    _swapChain->present();
}

void up::ShellApp::_drawUI() {
    _drawImgui.beginFrame();
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Potato")) {
            if (ImGui::MenuItem("Quit")) {
                _running = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem(!_paused ? "Pause" : "Play")) {
                _paused = !_paused;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (ImGui::Begin("Camera")) {
        auto pos = _camera.position();
        auto view = _camera.view();
        auto right = _camera.right();
        auto up = _camera.up();
        ImGui::InputFloat3("Position", &pos.x);
        ImGui::InputFloat3("View", &view.x);
        ImGui::InputFloat3("Right", &right.x);
        ImGui::InputFloat3("Up", &up.x);
        _camera.lookAt(pos, pos + view, up);
        if (ImGui::Button("Fly")) {
            _cameraController = new_box<FlyCameraController>(_camera);
        }
        else if (ImGui::Button("ArcBall")) {
            _cameraController = new_box<ArcBallCameraController>(_camera);
        }
    }
    ImGui::End();

    if (ImGui::Begin("Statistics")) {
        auto micro = std::chrono::duration_cast<std::chrono::microseconds>(_lastFrameDuration).count();

        fixed_string_writer<128> buffer;
        format_into(buffer, "{}us", micro);
        ImGui::LabelText("Frametime", "%s", buffer.c_str());
        buffer.clear();
        format_into(buffer, "{}", 1.f / _lastFrameTime);
        ImGui::LabelText("FPS", "%s", buffer.c_str());
    }
    ImGui::End();
}

void up::ShellApp::_errorDialog(zstring_view message) {
    _logger.error("Fatal error: {}", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), _window.get());
}

bool up::ShellApp::_loadConfig(zstring_view path) {
    auto stream = _fileSystem.openRead(path, FileOpenMode::Text);
    if (!stream) {
        _logger.error("Failed to open `{}'", path.c_str());
        return false;
    }

    nlohmann::json jsonRoot;
    IOResult rs = readJson(stream, jsonRoot);
    if (!jsonRoot.is_object()) {
        _logger.error("Failed to parse file `{}': {}", path, rs);
        return false;
    }

    auto jsonResourceDir = jsonRoot["resourceDir"];

    if (jsonResourceDir.is_string()) {
        _resourceDir = jsonResourceDir.get<string>();
    }

    return true;
}
