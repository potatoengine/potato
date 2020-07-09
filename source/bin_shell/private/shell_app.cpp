// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "components.h"
#include "scene.h"
#include "editors/filetree_editor.h"
#include "editors/game_editor.h"
#include "editors/scene_editor.h"

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
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/platform.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_writer.h"
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
#include <nfd.h>

up::shell::ShellApp::ShellApp() : _universe(new_box<Universe>()), _logger("shell") {}

up::shell::ShellApp::~ShellApp() {
    _drawImgui.releaseResources();

    _renderer.reset();
    _uiRenderCamera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::shell::ShellApp::initialize() {
    zstring_view configPath = "shell.config.json";
    if (fs::fileExists(configPath)) {
        _loadConfig(configPath);
    }

    if (_editorResourcePath.empty()) {
        _errorDialog("No editor resource path specified");
        return 1;
    }

    _resourceLoader.setCasPath(path::join(_editorResourcePath, ".library", "cache"));

    string manifestPath = path::join(_editorResourcePath, ".library", "manifest.txt");
    if (auto [rs, manifestText] = fs::readText(manifestPath); rs == IOResult{}) {
        if (!ResourceManifest::parseManifest(manifestText, _resourceLoader.manifest())) {
            _errorDialog("Failed to parse resource manifest");
            return 1;
        }
    }
    else {
        _errorDialog("Failed to load resource manifest");
        return 1;
    }

    constexpr int default_width = 1024;
    constexpr int default_height = 768;

    _window = SDL_CreateWindow(
        "loading",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        default_width,
        default_height,
        SDL_WINDOW_RESIZABLE);
    if (_window == nullptr) {
        _errorDialog("Could not create window");
    }
    _updateTitle();

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);

    if (SDL_GetWindowWMInfo(_window.get(), &wmInfo) != SDL_TRUE) {
        _errorDialog("Could not get window info");
        return 1;
    }

    _audio = AudioEngine::create(_resourceLoader);

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

    _loader = new_box<DefaultLoader>(_resourceLoader, _device);
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

    auto imguiVertShader = _loader->loadShaderSync("shaders/imgui.hlsl"_zsv, "vertex"_sv);
    auto imguiPixelShader = _loader->loadShaderSync("shaders/imgui.hlsl"_zsv, "pixel"_sv);
    if (imguiVertShader == nullptr || imguiPixelShader == nullptr) {
        _errorDialog("Failed to load imgui shaders");
        return 1;
    }

    _drawImgui.bindShaders(std::move(imguiVertShader), std::move(imguiPixelShader));
    auto fontStream = _resourceLoader.openAsset("fonts/roboto/Roboto-Regular.ttf");
    if (!fontStream) {
        _errorDialog("Failed to open Roboto-Regular font");
        return 1;
    }
    _drawImgui.loadFont(std::move(fontStream));

    fontStream = _resourceLoader.openAsset("fonts/fontawesome5/fa-solid-900.ttf");
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

    return 0;
}

bool up::shell::ShellApp::_selectAndLoadProject(zstring_view defaultPath) {
    nfdchar_t* selectedPath = nullptr;
    string folder = path::normalize(path::parent(defaultPath), path::Separator::Native);
    auto const result = NFD_OpenDialog("popr", folder.c_str(), &selectedPath);
    if (result != NFD_OKAY) {
        return false;
    }
    bool success = _loadProject(selectedPath);
    free(selectedPath); // NOLINT(cppcoreguidelines-no-malloc)
    return success;
}

bool up::shell::ShellApp::_loadProject(zstring_view path) {
    _project = Project::loadFromFile(path);
    if (_project == nullptr) {
        _errorDialog("Could not load project file");
        return false;
    }

    _projectName = path::filebasename(path);

    _editors.clear();
    _editors.push_back(createFileTreeEditor(_editorResourcePath, [this](zstring_view name) { _onFileOpened(name); }));

    _updateTitle();

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
        imguiIO.DeltaTime = _lastFrameTime;

        _processEvents();

        if (_openProject && !_closeProject) {
            _openProject = false;
            if (!_selectAndLoadProject(path::join(_editorResourcePath, "..", "resources", "sample.popr"))) {
                return;
            }
        }

        SDL_GetWindowSize(_window.get(), &width, &height);
        imguiIO.DisplaySize.x = static_cast<float>(width);
        imguiIO.DisplaySize.y = static_cast<float>(height);

        _drawImgui.beginFrame();

        _displayUI();
        _tick();

        _drawImgui.endFrame();

        _render();

        if (_closeProject) {
            _closeProject = false;
            _editors.clear();
            _project = nullptr;
            _updateTitle();
        }

        for (auto it = _editors.begin(); it != _editors.end();) {
            if (it->get()->isClosed()) {
                it = _editors.erase(it);
            }
            else {
                ++it;
            }
        }

        auto endFrame = std::chrono::high_resolution_clock::now();
        _lastFrameDuration = endFrame - now;
        _lastFrameTime = static_cast<float>(_lastFrameDuration.count() * nano_to_seconds);
        now = endFrame;
    }
}

void up::shell::ShellApp::quit() {
    _running = false;
}

void up::shell::ShellApp::_onWindowClosed() {
    quit();
}

void up::shell::ShellApp::_onWindowSizeChanged() {
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    _uiRenderCamera->resetBackBuffer(nullptr);
    _renderer->commandList().clear();
    _swapChain->resizeBuffers(width, height);
    _uiRenderCamera->resetBackBuffer(_swapChain->getBuffer(0));
}

void up::shell::ShellApp::_updateTitle() {
    static constexpr char appName[] = "Potato Shell";

    if (_project == nullptr) {
        SDL_SetWindowTitle(_window.get(), appName);
        return;
    }

    string_writer title;
    format_append(title, "{} [{}]", appName, _projectName);
    SDL_SetWindowTitle(_window.get(), title.c_str());
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

void up::shell::ShellApp::_tick() {
    for (auto& editor : _editors) {
        editor->tick(_lastFrameTime);
    }
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
        if (ImGui::BeginMenu("Potato")) {
            if (ImGui::BeginMenu("New")) {
                if (ImGui::MenuItem("Scene", nullptr, false, _project != nullptr)) {
                    _createScene();
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem(as_char(u8"\uf542 Open Project..."))) {
                _openProject = true;
                _closeProject = true;
            }
            if (ImGui::MenuItem(as_char(u8"\uf057 Close Project"), nullptr, false, _project != nullptr)) {
                _closeProject = true;
            }
            if (ImGui::MenuItem(as_char(u8"\uf52b Quit"), "ESC")) {
                _running = false;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Actions")) {
            ImGui::EndMenu();
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
    auto const windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.z - rect.x, rect.w - rect.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("MainWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(1);
    ImGui::DockSpace(mainDockId, {}, ImGuiDockNodeFlags_NoWindowMenuButton, &_documentWindowClass);
    ImGui::End();

    for (auto index : sequence(_editors.size())) {
        ImGui::SetNextWindowDockID(mainDockId, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowClass(&_documentWindowClass);
        _editors[index]->render(*_renderer);
    }
}

void up::shell::ShellApp::_errorDialog(zstring_view message) {
    _logger.error("Fatal error: {}", message);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal error", message.c_str(), _window.get());
}

bool up::shell::ShellApp::_loadConfig(zstring_view path) {
    auto [rs, jsonRoot] = readJson(path);
    if (rs != IOResult{} || !jsonRoot.is_object()) {
        _logger.error("Failed to parse file `{}': {}", path, rs);
        return false;
    }

    auto const editorResourcePath = jsonRoot["editorResourcePath"];

    if (editorResourcePath.is_string()) {
        _editorResourcePath = editorResourcePath.get<string>();
    }

    return true;
}

void up::shell::ShellApp::_onFileOpened(zstring_view filename) {
    if (path::extension(filename) == ".scene") {
        _createScene();
    }
}

void up::shell::ShellApp::_createScene() {
    auto material = _loader->loadMaterialSync("materials/full.mat");
    if (material == nullptr) {
        _errorDialog("Failed to load basic material");
        return;
    }

    auto mesh = _loader->loadMeshSync("meshes/cube.obj");
    if (mesh == nullptr) {
        _errorDialog("Failed to load cube mesh");
        return;
    }

    auto ding = _audio->loadSound("audio/kenney/highUp.mp3");
    if (ding == nullptr) {
        _errorDialog("Failed to load highUp mp3");
        return;
    }

    auto model = new_shared<Model>(std::move(mesh), std::move(material));

    auto scene = new_shared<Scene>(*_universe, *_audio);
    scene->create(model, ding);
    _editors.push_back(createSceneEditor(
        scene,
        [this] { return _universe->components(); },
        [this](rc<Scene> scene) { _createGame(std::move(scene)); }));
}

void up::shell::ShellApp::_createGame(rc<Scene> scene) {
    _editors.push_back(createGameEditor(std::move(scene)));
}
