// Copyright (C) 2018-2020 Sean Middleditch, all rights reserverd.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "scene.h"
#include "components.h"

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

namespace up::shell {
    extern auto createScenePanel(Renderer& renderer, Scene& scene) -> box<Panel>;
    extern auto createGamePanel(Renderer& renderer, Scene& scene) -> box<Panel>;
    extern auto createInspectorPanel(Scene& scene, Selection& selection) -> box<Panel>;
    extern auto createHierarchyPanel(Scene& scene, Selection& selection) -> box<Panel>;
} // namespace up::shell

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

    _window = SDL_CreateWindow("Potato Shell", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_RESIZABLE);
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

    _selection.select(_scene->main());

    auto imguiVertShader = _renderer->loadShaderSync("resources/shaders/imgui.vs_5_0.cbo");
    auto imguiPixelShader = _renderer->loadShaderSync("resources/shaders/imgui.ps_5_0.cbo");
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

    _documents.push_back(shell::createScenePanel(*_renderer, *_scene));
    _documents.push_back(shell::createGamePanel(*_renderer, *_scene));
    _documents.push_back(shell::createInspectorPanel(*_scene, _selection));
    _documents.push_back(shell::createHierarchyPanel(*_scene, _selection));

    auto& registry = ComponentRegistry::defaultRegistry();
    registry.registerComponent(&ComponentMeta::get<components::Position>());
    registry.registerComponent(&ComponentMeta::get<components::Rotation>());
    registry.registerComponent(&ComponentMeta::get<components::Transform>());
    registry.registerComponent(&ComponentMeta::get<components::Mesh>());
    registry.registerComponent(&ComponentMeta::get<components::Wave>());
    registry.registerComponent(&ComponentMeta::get<components::Spin>());

    return 0;
}

void up::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    auto now = std::chrono::high_resolution_clock::now();
    _lastFrameDuration = now - now;

    int width = 0;
    int height = 0;

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
            case ImGuiMouseCursor_NotAllowed:
                _cursor.reset(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO));
                break;
            default:
                _cursor.reset(SDL_GetDefaultCursor());
                break;
            }
            SDL_SetCursor(_cursor.get());
        }
    }

    SDL_Event ev;
    while (_running && SDL_PollEvent(&ev) > 0) {
        switch (ev.type) {
        case SDL_QUIT:
            quit();
            break;
        case SDL_WINDOWEVENT:
            switch (ev.window.event) {
            case SDL_WINDOWEVENT_CLOSE:
                _onWindowClosed();
                break;
            case SDL_WINDOWEVENT_MAXIMIZED:
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                _onWindowSizeChanged();
                break;
            case SDL_WINDOWEVENT_ENTER:
            case SDL_WINDOWEVENT_EXPOSED:
                break;
            }
            _drawImgui.handleEvent(ev);
            break;
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_KEYDOWN:
        case SDL_MOUSEWHEEL:
        default:
            _drawImgui.handleEvent(ev);
            break;
        }
    }
}

void up::ShellApp::_tick() {
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

    _renderer->beginFrame();
    auto ctx = _renderer->context();

    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
    _uiRenderCamera->beginFrame(ctx, {}, glm::identity<glm::mat4x4>());
    _drawImgui.render(ctx);
    _renderer->endFrame(_lastFrameTime);

    _swapChain->present();
}

void up::ShellApp::_displayUI() {
    auto& imguiIO = ImGui::GetIO();

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
            for (auto const& doc : _documents) {
                if (ImGui::MenuItem(doc->displayName().c_str(), nullptr, doc->enabled(), true)) {
                    doc->enabled(!doc->enabled());
                }
            }
            ImGui::EndMenu();
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Spacing();

        if (ImGui::MenuItem(as_char(_scene->playing() ? u8"\uf04c Pause" : u8"\uf04b Play"), "F5")) {
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
    if (ImGui::Begin("MainWindow", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
        auto dockSize = ImGui::GetContentRegionAvail();

        auto const dockId = ImGui::GetID("MainDockspace");

        if (ImGui::DockBuilderGetNode(dockId) == nullptr) {
            ImGui::DockBuilderRemoveNode(dockId);
            ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockId, dockSize);

            auto const centralDockId = ImGui::DockBuilderAddNode(dockId, ImGuiDockNodeFlags_None);

            auto contentDockId = centralDockId;
            auto inspectedDockId = ImGui::DockBuilderSplitNode(dockId, ImGuiDir_Right, 0.25f, nullptr, &contentDockId);
            auto const hierarchyDockId = ImGui::DockBuilderSplitNode(inspectedDockId, ImGuiDir_Down, 0.65f, nullptr, &inspectedDockId);

            ImGui::DockBuilderDockWindow(as_char(u8"\uf085 Inspector"), inspectedDockId);
            ImGui::DockBuilderDockWindow("Hierarchy", hierarchyDockId);
            ImGui::DockBuilderDockWindow("ScenePanel", contentDockId);
            ImGui::DockBuilderDockWindow("GamePanel", contentDockId);

            ImGui::DockBuilderFinish(dockId);
        }

        ImGui::DockSpace(dockId, {}, ImGuiDockNodeFlags_None);

        if (ImGui::Begin("Statistics", nullptr, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_AlwaysAutoResize)) {
            auto const contentSize = ImGui::GetContentRegionAvail();
            ImGui::SetWindowPos(ImVec2(io.DisplaySize.x - contentSize.x - 20, rect.y));

            auto micro = std::chrono::duration_cast<std::chrono::microseconds>(_lastFrameDuration).count();

            fixed_string_writer<128> buffer;
            format_append(buffer, "{}us | FPS {}", micro, static_cast<int>(1.f / _lastFrameTime));
            ImGui::Text("%s", buffer.c_str());
        }
        ImGui::End();
    }
    ImGui::End();
    ImGui::PopStyleVar(1);

    for (auto const& doc : _documents) {
        doc->ui();
    }
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
