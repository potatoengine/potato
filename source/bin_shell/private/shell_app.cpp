// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "scene.h"

#include "potato/spud/box.h"
#include "potato/spud/platform.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/path.h"
#include "potato/runtime/json.h"
#include "potato/runtime/native.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/renderer.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
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
#include <glm/gtx/functions.hpp>

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

    if (SDL_GetWindowWMInfo(_window.get(), &wmInfo) != SDL_TRUE) {
        _errorDialog("Could not get window info");
        return 1;
    }

#if defined(UP_GPU_ENABLE_D3D11)
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

    _renderCamera = new_box<RenderCamera>();
    _renderCamera->resetBackBuffer(_swapChain->getBuffer(0));

    auto material = _renderer->loadMaterialSync("resources/materials/full.mat");
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
    auto fontStream = _fileSystem.openRead("resources/fonts/fontawesome5/fa-solid-900.ttf");
    if (!fontStream) {
        _errorDialog("Failed to open FontAwesome font");
        return 1;
    }
    if (!_drawImgui.loadFontAwesome5(std::move(fontStream))) {
        _errorDialog("Failed to load FontAwesome font");
        return 1;
    }
    _drawImgui.createResources(*_device);

    _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
    _cameraController = new_box<ArcBallCameraController>(_camera);

    return 0;
}

void up::ShellApp::run() {
    auto now = std::chrono::high_resolution_clock::now();
    _lastFrameDuration = now - now;

    while (isRunning()) {
        _processEvents();
        _displayUI();
        _tick();
        _render();

        auto endFrame = std::chrono::high_resolution_clock::now();
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
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _renderCamera->resetBackBuffer(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _renderCamera->resetBackBuffer(_swapChain->getBuffer(0));
    _sceneBuffer = nullptr;
}

void up::ShellApp::_processEvents() {
    auto& imguiIO = ImGui::GetIO();

    _inputState->relativeMotion = {0, 0, 0};
    _inputState->relativeMovement = {0, 0, 0};

    SDL_Event ev;
    while (SDL_PollEvent(&ev) > 0) {
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
            if (ev.key.keysym.scancode == SDL_SCANCODE_F && (ev.key.keysym.mod & KMOD_CTRL) != 0) {
                _cameraController = new_box<FlyCameraController>(_camera);
            }
            if (ev.key.keysym.scancode == SDL_SCANCODE_B && (ev.key.keysym.mod & KMOD_CTRL) != 0) {
                _cameraController = new_box<ArcBallCameraController>(_camera);
            }
            if (ev.key.keysym.scancode == SDL_SCANCODE_F5) {
                _paused = !_paused;
            }
            break;
        case SDL_MOUSEWHEEL:
            if (!imguiIO.WantCaptureMouse) {
                _inputState->relativeMotion.z += (ev.wheel.y > 0.f ? 1.f : ev.wheel.y < 0 ? -1.f : 0.f) * (ev.wheel.direction == SDL_MOUSEWHEEL_FLIPPED ? -1.f : 1.f);
            }
            break;
        }
        _drawImgui.handleEvent(ev);
    }

    if (!imguiIO.WantCaptureKeyboard) {
        auto keys = SDL_GetKeyboardState(nullptr);
        _inputState->relativeMovement = {static_cast<float>(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                                         static_cast<float>(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_C]),
                                         static_cast<float>(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S])};
    }
}

void up::ShellApp::_tick() {
    _cameraController->apply(_camera, _inputState->relativeMovement, _inputState->relativeMotion, _lastFrameTime);

    if (!_paused) {
        _scene->tick(_lastFrameTime);
    }

    _scene->flush();
}

void up::ShellApp::_render() {
    GpuViewportDesc viewport;
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);

    if (_sceneBuffer != nullptr) {
        _renderer->beginFrame();
        auto ctx = _renderer->context();

        _renderCamera->resetBackBuffer(_sceneBuffer);
        if (_grid) {
            _drawGrid();
        }
        _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
        _scene->render(ctx);
        _renderer->flushDebugDraw(_lastFrameTime);
        _renderCamera->endFrame(ctx);
        _renderer->endFrame(_lastFrameTime);
    }

    {
        _renderer->beginFrame();
        auto ctx = _renderer->context();

        _renderCamera->resetBackBuffer(_swapChain->getBuffer(0));
        _renderCamera->beginFrame(ctx, _camera.position(), _camera.matrix());
        _drawImgui.endFrame(*_device, _renderer->commandList());
        _renderCamera->endFrame(ctx);
        _renderer->endFrame(_lastFrameTime);
    }

    _swapChain->present();
}

namespace {
    class ImGuiComponentReflector final : public up::ComponentReflector {
    protected:
        void onField(up::zstring_view name) override {
            _name = name;
        }

        void onValue(int& value) override {
            ImGui::InputInt(_name.c_str(), &value);
        }

        void onValue(float& value) override {
            ImGui::InputFloat(_name.c_str(), &value);
        }

        void onValue(up::EntityId value) override {
            ImGui::LabelText(_name.c_str(), "%u", (unsigned)value);
        }

        void onValue(glm::vec3& value) override {
            ImGui::InputFloat3(_name.c_str(), &value.x);
        }

    private:
        up::zstring_view _name;
    };
} // namespace

void up::ShellApp::_displayUI() {
    auto& imguiIO = ImGui::GetIO();
    ImVec2 menuSize;

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    imguiIO.DisplaySize.x = static_cast<float>(width);
    imguiIO.DisplaySize.y = static_cast<float>(height);

    _drawImgui.beginFrame();
    if (ImGui::BeginMainMenuBar()) {
        menuSize = ImGui::GetWindowSize();

        if (ImGui::BeginMenu(u8"\uf094 Potato")) {
            if (ImGui::MenuItem(u8"\uf52b Quit", "ESC")) {
                _running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"\uf06e View")) {
            if (ImGui::BeginMenu("Camera")) {
                if (ImGui::MenuItem("Fly", "ctrl-f")) {
                    _cameraController = new_box<FlyCameraController>(_camera);
                }
                if (ImGui::MenuItem("ArcBall", "ctrl-b")) {
                    _cameraController = new_box<ArcBallCameraController>(_camera);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Grid")) {
                    _grid = !_grid;
                }
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Reset")) {
                _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
                _cameraController = new_box<ArcBallCameraController>(_camera);
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"\uf2d2 Windows")) {

            if (ImGui::MenuItem("Inspector")) {
                _showInspector = !_showInspector;
                _sceneBuffer = nullptr;
            }

            ImGui::EndMenu();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::MenuItem(!_paused ? u8"\uf04c Pause" : u8"\uf04b Play", "F5")) {
            _paused = !_paused;
        }

        ImGui::EndMainMenuBar();
    }

    _drawDocumentsUI({0, menuSize.y, imguiIO.DisplaySize.x, imguiIO.DisplaySize.y});

    if (ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize)) {
        auto const fpSize = ImGui::GetWindowSize();
        ImGui::SetWindowPos(ImVec2(imguiIO.DisplaySize.x - fpSize.x - 20, menuSize.y));

        auto micro = std::chrono::duration_cast<std::chrono::microseconds>(_lastFrameDuration).count();

        fixed_string_writer<128> buffer;
        format_append(buffer, "{}us | FPS {}", micro, static_cast<int>(1.f / _lastFrameTime));
        ImGui::Text("%s", buffer.c_str());
    }
    ImGui::End();
}

void up::ShellApp::_drawDocumentsUI(glm::vec4 rect) {
    auto& io = ImGui::GetIO();

    auto const height = rect.w - rect.y;
    auto const width = rect.z - rect.x;
    auto const pos = glm::vec2{rect.x, rect.y};

    ImGui::SetNextWindowPos(ImVec2(0, pos.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar)) {

        if (ImGui::BeginTabBar("DocumentTabs", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_Reorderable)) {
            if (ImGui::BeginTabItem("SceneTab", nullptr, ImGuiTabItemFlags_None)) {
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

                auto const contentSize = ImGui::GetContentRegionAvail();

                float inspectorVisibleWidth = 0;
                if (_showInspector) {
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
                    if (ImGui::BeginChild(u8"\uf085 Inspector", ImVec2(_inspectorWidth, contentSize.y))) {
                        _scene->world().interrogateEntity(_scene->main(), [](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
                            if (ImGui::CollapsingHeader(meta->name.c_str())) {
                                ImGuiComponentReflector ref;
                                meta->reflect(data, ref);
                            }
                        });
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar(1);

                    ImGui::SameLine();
                    ImGui::InvisibleButton("vsplitter", ImVec2(4.0f, contentSize.y));
                    if (ImGui::IsItemActive()) {
                        _inspectorWidth = glm::clamp(_inspectorWidth + io.MouseDelta.x, 30.f, contentSize.x - 80.f);
                    }
                    ImGui::SameLine();

                    inspectorVisibleWidth = _inspectorWidth + 4;
                }

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
                ImVec2 sceneSize = {width - inspectorVisibleWidth, contentSize.y};
                if (ImGui::BeginChild("SceneView", sceneSize, false)) {
                    glm::vec3 bufferSize = {0, 0, 0};
                    if (_sceneBuffer != nullptr) {
                        bufferSize = _sceneBuffer->dimensions();
                    }
                    if (bufferSize.x != sceneSize.x || bufferSize.y != sceneSize.y) {
                        _resizeSceneView({sceneSize.x, sceneSize.y});
                    }

                    auto const pos = ImGui::GetCursorPos();
                    ImGui::Image(_sceneBufferView.get(), sceneSize);
                    ImGui::SetCursorPos(pos);
                    ImGui::InvisibleButton("SceneInteract", sceneSize);
                    if (ImGui::IsItemActive()) {
                        _inputState->relativeMotion.x = io.MouseDelta.x / sceneSize.x;
                        _inputState->relativeMotion.y = io.MouseDelta.y / sceneSize.y;
                    }
                }
                ImGui::EndChild();
                ImGui::PopStyleVar(1);

                ImGui::PopStyleVar(1);
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("EmptyTab", nullptr, ImGuiTabItemFlags_None)) {
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
    ImGui::PopStyleVar(1);
}

void up::ShellApp::_drawGrid() {
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

void up::ShellApp::_resizeSceneView(glm::ivec2 size) {
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _sceneBuffer = _device->createTexture2D(desc, {});

    _sceneBufferView = _device->createShaderResourceView(_sceneBuffer.get());
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
