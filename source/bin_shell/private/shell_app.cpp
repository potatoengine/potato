// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "components.h"
#include "scene.h"

#include "potato/ecs/query.h"
#include "potato/ecs/world.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/draw_imgui.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/model.h"
#include "potato/render/renderer.h"
#include "potato/render/shader.h"
#include "potato/tools/project.h"
#include "potato/runtime/json.h"
#include "potato/runtime/native.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/platform.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/functions.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/vec3.hpp>
#include <nlohmann/json.hpp>
#include <SDL.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>

namespace up::shell {
    extern auto createSceneDocument(rc<Scene> scene) -> box<Document>;

    extern auto createScenePanel(Renderer& renderer, rc<Scene> scene) -> box<Panel>;
    extern auto createGamePanel(Renderer& renderer, rc<Scene> scene) -> box<Panel>;
    extern auto createInspectorPanel(rc<Scene> scene, Selection& selection, delegate<view<ComponentMeta>()> components) -> box<Panel>;
    extern auto createHierarchyPanel(rc<Scene> scene, Selection& selection) -> box<Panel>;
} // namespace up::shell

up::shell::ShellApp::ShellApp() : _universe(new_box<Universe>()), _logger("shell") {}

up::shell::ShellApp::~ShellApp() {
    _panels.clear();

    _drawImgui.releaseResources();

    _renderer.reset();
    _uiRenderCamera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::shell::ShellApp::initialize() {
    zstring_view configPath = "shell.config.json";
    if (_fileSystem.fileExists(configPath)) {
        _loadConfig(configPath);
    }

    _fileSystem.currentWorkingDirectory(_editorResourcePath);

    constexpr int default_width = 1024;
    constexpr int default_height = 768;

    _window = SDL_CreateWindow("Potato Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, default_width, default_height, SDL_WINDOW_RESIZABLE);
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

    _loader = new_box<DefaultLoader>(_fileSystem, _device);
    _renderer = new_box<Renderer>(*_loader, _device);

#if UP_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        _errorDialog("Failed to create swap chain");
        return 1;
    }

    _uiRenderCamera = new_box<RenderCamera>();
    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));

    _documentWindowClass.ClassId = ImHashStr("DocumentClass");
    _documentWindowClass.DockingAllowUnclassed = false;
    _documentWindowClass.DockingAlwaysTabBar = true;

    auto imguiVertShader = _loader->loadShaderSync("resources/shaders/imgui.vs_5_0.cbo");
    auto imguiPixelShader = _loader->loadShaderSync("resources/shaders/imgui.ps_5_0.cbo");
    if (imguiVertShader == nullptr || imguiPixelShader == nullptr) {
        _errorDialog("Failed to load imgui shaders");
        return 1;
    }

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    auto fontStream = _fileSystem.openRead("resources/fonts/roboto/Roboto-Regular.ttf");
    if (!fontStream) {
        _errorDialog("Failed to open Roboto-Regular font");
        return 1;
    }
    _drawImgui.loadFont(std::move(fontStream));

    fontStream = _fileSystem.openRead("resources/fonts/fontawesome5/fa-solid-900.ttf");
    if (!fontStream) {
        _errorDialog("Failed to open FontAwesome font");
        return 1;
    }
    if (!_drawImgui.loadFontAwesome5(std::move(fontStream))) {
        _errorDialog("Failed to load FontAwesome font");
        return 1;
    }

    _drawImgui.createResources(*_device);

    _universe = new_box<Universe>();

    _universe->registerComponent<components::Position>("Position");
    _universe->registerComponent<components::Rotation>("Rotation");
    _universe->registerComponent<components::Transform>("Transform");
    _universe->registerComponent<components::Mesh>("Mesh");
    _universe->registerComponent<components::Wave>("Wave");
    _universe->registerComponent<components::Spin>("Spin");
    _universe->registerComponent<components::Ding>("Ding");

    if (!_loadProject(path::join(_editorResourcePath, "..", "resources", "sample.popr"))) {
        return 1;
    }

    _panels.push_back(shell::createScenePanel(*_renderer, _scene));
    _panels.push_back(shell::createGamePanel(*_renderer, _scene));
    _panels.push_back(shell::createInspectorPanel(_scene, _selection, [this] { return _universe->components(); }));
    _panels.push_back(shell::createHierarchyPanel(_scene, _selection));

    return 0;
}

bool up::shell::ShellApp::_loadProject(zstring_view path) {
    _project = Project::loadFromFile(_fileSystem, path);
    if (_project == nullptr) {
        _errorDialog("Could not load project file");
        return false;
    }

    _fileSystem.currentWorkingDirectory(_project->targetPath());

    auto material = _loader->loadMaterialSync("resources/materials/full.mat");
    if (material == nullptr) {
        _errorDialog("Failed to load basic material");
        return false;
    }

    auto mesh = _loader->loadMeshSync("resources/meshes/cube.model");
    if (mesh == nullptr) {
        _errorDialog("Failed to load cube mesh");
        return false;
    }

    _audio = AudioEngine::create(_fileSystem);

    _ding = _audio->loadSound("resources/audio/kenney/highUp.mp3");
    if (_ding == nullptr) {
        _errorDialog("Failed to load coin mp3");
        return false;
    }

    _scene = new_shared<Scene>(*_universe);
    _scene->create(new_shared<Model>(std::move(mesh), std::move(material)), _ding);

    _documents.clear();
    _documents.push_back(createSceneDocument(_scene));

    _selection.select(_scene->main());

    return true;
}

void up::shell::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    auto now = std::chrono::high_resolution_clock::now();
    _lastFrameDuration = now - now;

    int width = 0;
    int height = 0;

    constexpr double nano_to_seconds = 1.0 / 1000000000.0;

    while (isRunning()) {
        _processEvents();

        SDL_GetWindowSize(_window.get(), &width, &height);
        imguiIO.DisplaySize.x = static_cast<float>(width);
        imguiIO.DisplaySize.y = static_cast<float>(height);

        _drawImgui.beginFrame();

        _displayUI();
        _tick();

        _drawImgui.endFrame();

        _render();

        auto endFrame = std::chrono::high_resolution_clock::now();
        _lastFrameDuration = endFrame - now;
        _lastFrameTime = static_cast<float>(_lastFrameDuration.count() * nano_to_seconds);
        now = endFrame;
    }
}

void up::shell::ShellApp::quit() { _running = false; }

void up::shell::ShellApp::_onWindowClosed() { quit(); }

void up::shell::ShellApp::_onWindowSizeChanged() {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _uiRenderCamera->resetBackBuffer(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
}

void up::shell::ShellApp::_processEvents() {
    auto& io = ImGui::GetIO();

    SDL_SetRelativeMouseMode(ImGui::IsCaptureRelativeMouseMode() ? SDL_TRUE : SDL_FALSE);
    SDL_CaptureMouse(io.WantCaptureMouse || ImGui::IsCaptureRelativeMouseMode() ? SDL_TRUE : SDL_FALSE);

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
            case ImGuiMouseCursor_TextInput: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM)); break;
            case ImGuiMouseCursor_ResizeAll: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL)); break;
            case ImGuiMouseCursor_ResizeNS: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS)); break;
            case ImGuiMouseCursor_ResizeEW: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE)); break;
            case ImGuiMouseCursor_ResizeNESW: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW)); break;
            case ImGuiMouseCursor_ResizeNWSE: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE)); break;
            case ImGuiMouseCursor_Hand: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND)); break;
            case ImGuiMouseCursor_NotAllowed: _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO)); break;
            default: _cursor.reset(SDL_GetDefaultCursor()); break;
            }
            SDL_SetCursor(_cursor.get());
        }
    }

    SDL_Event ev;
    while (_running && SDL_PollEvent(&ev) > 0) {
        switch (ev.type) {
        case SDL_QUIT: quit(); break;
        case SDL_WINDOWEVENT:
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_CLOSE: _onWindowClosed(); break;
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED: _onWindowSizeChanged(); break;
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_EXPOSED: break;
            }
            _drawImgui.handleEvent(ev);
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_KEYDOWN:
        case SDL_MOUSEWHEEL:
        default: _drawImgui.handleEvent(ev); break;
        }
    }
}

void up::shell::ShellApp::_tick() {
    _scene->tick(_lastFrameTime, *_audio);
    _scene->flush();
}

void up::shell::ShellApp::_render() {
    GpuViewportDesc viewport;
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);

    _renderer->beginFrame();
    auto ctx = _renderer->context();

    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
    _uiRenderCamera->beginFrame(ctx, {}, glm::identity<glm::mat4x4>());
    _drawImgui.render(ctx);
    _renderer->endFrame(_lastFrameTime);

    _swapChain->present();
}

void up::shell::ShellApp::_displayUI() {
    auto& imguiIO = ImGui::GetIO();

    _displayMainMenu();

    ImVec2 menuSize;
    if (ImGui::BeginMainMenuBar()) {
        menuSize = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }

    _displayDocuments({0, menuSize.y, imguiIO.DisplaySize.x, imguiIO.DisplaySize.y});
}

void up::shell::ShellApp::_displayMainMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu(as_char(u8"\uf094 Potato"))) {
            if (ImGui::MenuItem(as_char(u8"\uf52b Quit"), "ESC")) {
                _running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(as_char(u8"\uf06e View"))) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu(as_char(u8"\uf2d2 Windows"))) {
            for (auto const& doc : _panels) {
                if (ImGui::MenuItem(doc->displayName().c_str(), nullptr, doc->enabled(), true)) {
                    doc->enabled(!doc->enabled());
                }
            }
            ImGui::EndMenu();
        }

        {
            auto const text = as_char(_scene->playing() ? u8"\uf04c Pause" : u8"\uf04b Play");
            auto const xPos = ImGui::GetWindowSize().x * 0.5f - ImGui::CalcTextSize(text).x * 0.5f - ImGui::GetStyle().ItemInnerSpacing.x;
            ImGui::SetCursorPosX(xPos);
            if (ImGui::MenuItem(as_char(_scene->playing() ? u8"\uf04c Pause" : u8"\uf04b Play"), "F5")) {
                _scene->playing(!_scene->playing());
            }
            ImGui::TextColored(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled), "Shift-ESC to release input");
        }

        {
            auto micro = std::chrono::duration_cast<std::chrono::microseconds>(_lastFrameDuration).count();

            fixed_string_writer<128> buffer;
            format_append(buffer, "{}us | FPS {}", micro, static_cast<int>(1.f / _lastFrameTime));
            auto const textWidth = ImGui::CalcTextSize(buffer.c_str()).x;
            ImGui::SameLine(ImGui::GetWindowSize().x - textWidth - 2 * ImGui::GetStyle().FramePadding.x);
            ImGui::Text("%s", buffer.c_str());
        }

        ImGui::EndMainMenuBar();
    }
}

void up::shell::ShellApp::_displayDocuments(glm::vec4 rect) {
    auto const mainDockId = ImGui::GetID("DocumentsDockspace");
    auto const windowFlags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.z - rect.x, rect.w - rect.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("MainWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(1);
    ImGui::DockSpace(mainDockId, {}, ImGuiDockNodeFlags_NoSplit, &_documentWindowClass);
    ImGui::End();

    for (auto const& doc : _panels) {
        doc->ui();
    }

    for (auto const& doc : _documents) {
        ImGui::SetNextWindowDockID(mainDockId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_documentWindowClass);
        doc->render(*_renderer);
    }
}

void up::shell::ShellApp::_errorDialog(zstring_view message) {
    _logger.error("Fatal error: {}", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), _window.get());
}

bool up::shell::ShellApp::_loadConfig(zstring_view path) {
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

    auto const editorResourcePath = jsonRoot["editorResourcePath"];

    if (editorResourcePath.is_string()) {
        _editorResourcePath = editorResourcePath.get<string>();
    }

    return true;
}
