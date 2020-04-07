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
#include <imgui_internal.h>

#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/functions.hpp>

#include <nlohmann/json.hpp>

namespace {
    class SceneView : public up::shell::Document {
    public:
        explicit SceneView(up::GpuDevice& device, up::Scene& scene) : _device(device), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }
        virtual ~SceneView() = default;

        void render(up::Renderer& renderer, float frameTime) override;
        void ui() override;
        void handleEvent(SDL_Event const& ev) override;
        void tick(float deltaTime) override;

    private:
        void _drawGrid();
        void _resize(glm::ivec2 size);

    private:
        up::GpuDevice& _device;
        up::Scene& _scene;
        up::rc<up::GpuTexture> _buffer;
        up::box<up::GpuResourceView> _bufferView;
        up::box<up::RenderCamera> _renderCamera;
        up::Camera _camera;
        up::ArcBallCameraController _cameraController;
        glm::vec3 _relMotion{0, 0, 0};
        glm::vec3 _relMovement{0, 0, 0};
        bool _enableGrid = true;
        bool _isControllingCamera = false;
    };

    class GameView : public up::shell::Document {
    public:
        explicit GameView(up::GpuDevice& device, up::Scene& scene) : _device(device), _scene(scene), _cameraController(_camera) {
            _camera.lookAt({0, 10, 15}, {0, 0, 0}, {0, 1, 0});
        }
        virtual ~GameView() = default;

        void render(up::Renderer& renderer, float frameTime) override;
        void ui() override;
        void handleEvent(SDL_Event const& ev) override;
        void tick(float deltaTime) override;

    private:
        void _resize(glm::ivec2 size);

    private:
        up::GpuDevice& _device;
        up::Scene& _scene;
        up::rc<up::GpuTexture> _buffer;
        up::box<up::GpuResourceView> _bufferView;
        up::box<up::RenderCamera> _renderCamera;
        up::Camera _camera;
        up::FlyCameraController _cameraController;
        bool _isInputBound = false;
    };
}

void SceneView::render(up::Renderer& renderer, float frameTime) {
    if (_renderCamera == nullptr) {
        _renderCamera = up::new_box<up::RenderCamera>();
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

void SceneView::ui() {
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

    if (ImGui::Begin("SceneView", nullptr, ImGuiWindowFlags_None)) {
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
                _relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
                _relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar(1);
    }
    ImGui::End();
}

void SceneView::handleEvent(SDL_Event const& ev) {
}

void SceneView::tick(float deltaTime) {
    _cameraController.apply(_camera, _relMovement, _relMotion, deltaTime);
    _relMovement = {0, 0, 0};
    _relMotion = {0, 0, 0};
}

void SceneView::_drawGrid() {
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

    up::DebugDrawGrid grid;
    grid.axis2 = {0, 0, 1};
    grid.offset = {x, 0, z};
    grid.halfWidth = 1000;
    grid.spacing = spacing;
    grid.guidelineSpacing = guidelines;
    drawDebugGrid(grid);
}


void SceneView::_resize(glm::ivec2 size) {
    using namespace up;
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _buffer = _device.createTexture2D(desc, {});

    _bufferView = _device.createShaderResourceView(_buffer.get());
}

void GameView::render(up::Renderer& renderer, float frameTime) {
    if (_renderCamera == nullptr) {
        _renderCamera = up::new_box<up::RenderCamera>();
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

void GameView::ui() {
    SDL_CaptureMouse(_isInputBound ? SDL_TRUE : SDL_FALSE);
    SDL_SetRelativeMouseMode(_isInputBound ? SDL_TRUE : SDL_FALSE);

    if (ImGui::Begin("GameView", nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
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

void GameView::handleEvent(SDL_Event const& ev) {
    if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE && 0 != (ev.key.keysym.mod & KMOD_SHIFT)) {
        _isInputBound = false;
    }
}

void GameView::tick(float deltaTime) {
    _isInputBound = _isInputBound && _scene.playing();

    if (_isInputBound) {
        glm::vec3 relMotion = {0, 0, 0}, relMove = {0, 0, 0};

        auto& io = ImGui::GetIO();
        relMotion.x = io.MouseDelta.x / io.DisplaySize.x;
        relMotion.y = io.MouseDelta.y / io.DisplaySize.y;
        relMotion.z = io.MouseWheel > 0.f ? 1.f : io.MouseWheel < 0 ? -1.f : 0.f;

        auto keys = SDL_GetKeyboardState(nullptr);
        relMove= {static_cast<float>(keys[SDL_SCANCODE_D] - keys[SDL_SCANCODE_A]),
                        static_cast<float>(keys[SDL_SCANCODE_SPACE] - keys[SDL_SCANCODE_C]),
                        static_cast<float>(keys[SDL_SCANCODE_W] - keys[SDL_SCANCODE_S])};

        _cameraController.apply(_camera, relMove, relMotion, deltaTime);
    }
}

void GameView::_resize(glm::ivec2 size) {
    using namespace up;
    GpuTextureDesc desc;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.type = GpuTextureType::Texture2D;
    desc.width = size.x;
    desc.height = size.y;
    _buffer = _device.createTexture2D(desc, {});

    _bufferView = _device.createShaderResourceView(_buffer.get());
}

up::ShellApp::ShellApp() : _scene(new_box<Scene>()), _logger("shell") {}

up::ShellApp::~ShellApp() {
    _documents.clear();

    _drawImgui.releaseResources();

    _renderer.reset();
    _uiRenderCamera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::ShellApp::initialize() {
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

    _uiRenderCamera = new_box<RenderCamera>();
    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));

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

    _documents.push_back(new_box<SceneView>(*_device, *_scene));
    _documents.push_back(new_box<GameView>(*_device, *_scene));

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
    _uiRenderCamera->resetBackBuffer(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
}

void up::ShellApp::_processEvents() {
    auto& io = ImGui::GetIO();

    SDL_CaptureMouse(io.WantCaptureMouse ? SDL_TRUE : SDL_FALSE);

    auto const guiCursor = ImGui::GetMouseCursor();
    if (guiCursor != _lastCursor) {
        _lastCursor = guiCursor;
        SDL_ShowCursor(guiCursor != ImGuiMouseCursor_None ? SDL_TRUE : SDL_FALSE);
        if (guiCursor == ImGuiMouseCursor_Arrow) {
            SDL_SetCursor(SDL_GetDefaultCursor());
            _cursor.reset();
        }
        else {
            switch (guiCursor) {
            case ImGuiMouseCursor_TextInput:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM));
                break;
            case ImGuiMouseCursor_ResizeAll:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL));
                break;
            case ImGuiMouseCursor_ResizeNS:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS));
                break;
            case ImGuiMouseCursor_ResizeEW:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE));
                break;
            case ImGuiMouseCursor_ResizeNESW:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW));
                break;
            case ImGuiMouseCursor_ResizeNWSE:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE));
                break;
            case ImGuiMouseCursor_Hand:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND));
                break;
            default:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO));
                break;
            }
            SDL_SetCursor(_cursor.get());
        }
    }

    SDL_Event ev;
    while (SDL_PollEvent(&ev) > 0) {
        switch (ev.type) {
        case SDL_QUIT:
            return;
        case SDL_WINDOWEVENT:
            _drawImgui.handleEvent(ev);
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
            _drawImgui.handleEvent(ev);
            if (ev.key.keysym.scancode == SDL_SCANCODE_F5) {
                _scene->playing(!_scene->playing());
            }
            break;
        case SDL_MOUSEWHEEL:
            _drawImgui.handleEvent(ev);
            break;
        case SDL_MOUSEMOTION:
            _drawImgui.handleEvent(ev);
            break;
        case SDL_MOUSEBUTTONUP:
            _drawImgui.handleEvent(ev);
            break;
        default:
            _drawImgui.handleEvent(ev);
            break;
        }

        for (auto const& doc : _documents) {
            doc->handleEvent(ev);
        }
    }
}

void up::ShellApp::_tick() {
    for (auto const& doc : _documents) {
        doc->tick(_lastFrameTime);
    }

    _scene->tick(_lastFrameTime);

    _scene->flush();
}

void up::ShellApp::_render() {
    GpuViewportDesc viewport;
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);

    for (auto const& doc : _documents) {
        doc->render(*_renderer, _lastFrameTime);
    }

    _renderer->beginFrame();
    auto ctx = _renderer->context();

    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
    _uiRenderCamera->beginFrame(ctx, {}, glm::identity<glm::mat4x4>());
    _drawImgui.endFrame(*_device, _renderer->commandList());
    _renderer->endFrame(_lastFrameTime);

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

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    imguiIO.DisplaySize.x = static_cast<float>(width);
    imguiIO.DisplaySize.y = static_cast<float>(height);

    _drawImgui.beginFrame();
    _displayMainMenu();

    ImVec2 menuSize;
    if (ImGui::BeginMainMenuBar()) {
        menuSize = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }

    _displayDocuments({0, menuSize.y, imguiIO.DisplaySize.x, imguiIO.DisplaySize.y});
}

void up::ShellApp::_displayMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(u8"\uf094 Potato")) {
            if (ImGui::MenuItem(u8"\uf52b Quit", "ESC")) {
                _running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"\uf06e View")) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(u8"\uf2d2 Windows")) {

            if (ImGui::MenuItem("Inspector")) {
                _showInspector = !_showInspector;
            }

            ImGui::EndMenu();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::MenuItem(_scene->playing() ? u8"\uf04c Pause" : u8"\uf04b Play", "F5")) {
            _scene->playing(!_scene->playing());
        }

        ImGui::EndMainMenuBar();
    }
}

void up::ShellApp::_displayDocuments(glm::vec4 rect) {
    auto& io = ImGui::GetIO();

    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.z - rect.x, rect.w - rect.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground)) {
        auto dockSize = ImGui::GetContentRegionAvail();

        auto const dockId = ImGui::GetID("MainDockspace");

        if (ImGui::DockBuilderGetNode(dockId) == nullptr) {
            ImGui::DockBuilderRemoveNode(dockId);
            ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockId, dockSize);

            auto contentDockId = dockId;
            auto const paneDockId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Left, 0.25f, nullptr, &contentDockId);

            ImGui::DockBuilderDockWindow(u8"\uf085 Inspector", paneDockId);
            ImGui::DockBuilderDockWindow("SceneView", contentDockId);
            ImGui::DockBuilderDockWindow("GameView", contentDockId);

            ImGui::DockBuilderFinish(dockId);
        }

        ImGui::DockSpace(dockId, {}, ImGuiDockNodeFlags_AutoHideTabBar);

        if (ImGui::Begin(u8"\uf085 Inspector")) {
            _scene->world().interrogateEntity(_scene->main(), [](EntityId entity, ArchetypeId archetype, ComponentMeta const* meta, auto* data) {
                if (ImGui::CollapsingHeader(meta->name.c_str())) {
                    ImGuiComponentReflector ref;
                    meta->reflect(data, ref);
                }
            });
        }
        ImGui::End();

        if (ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize)) {
            auto const contentSize = ImGui::GetContentRegionAvail();
            ImGui::SetWindowPos(ImVec2(contentSize.x, contentSize.y));

            auto micro = std::chrono::duration_cast<std::chrono::microseconds>(_lastFrameDuration).count();

            fixed_string_writer<128> buffer;
            format_append(buffer, "{}us | FPS {}", micro, static_cast<int>(1.f / _lastFrameTime));
            ImGui::Text("%s", buffer.c_str());
        }
        ImGui::End();
                
        for (auto const& doc : _documents) {
            doc->ui();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar(1);
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
