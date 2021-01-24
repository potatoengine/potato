// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "shell_app.h"
#include "camera.h"
#include "camera_controller.h"
#include "components_schema.h"
#include "recon_messages_schema.h"
#include "scene.h"
#include "editors/asset_browser.h"
#include "editors/game_editor.h"
#include "editors/material_editor.h"
#include "editors/scene_editor.h"

#include "potato/audio/sound_resource.h"
#include "potato/ecs/query.h"
#include "potato/ecs/world.h"
#include "potato/editor/imgui_backend.h"
#include "potato/editor/imgui_ext.h"
#include "potato/render/camera.h"
#include "potato/render/context.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_factory.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/gpu_texture.h"
#include "potato/render/material.h"
#include "potato/render/renderer.h"
#include "potato/render/shader.h"
#include "potato/tools/desktop.h"
#include "potato/tools/project.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/delegate.h"
#include "potato/spud/platform.h"
#include "potato/spud/sequence.h"
#include "potato/spud/string_util.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

#include <nlohmann/json.hpp>
#include <SDL.h>
#include <SDL_keycode.h>
#include <SDL_messagebox.h>
#include <SDL_syswm.h>
#include <Tracy.hpp>
#include <chrono>
#include <imgui.h>
#include <imgui_internal.h>
#include <nfd.h>

// SDL includes X.h on Linux, which pollutes this name we care about
// FIXME: clean up this file for better abstractions to avoid header pollution problems
#if defined(Success)
#    undef Success
#endif

up::shell::ShellApp::ShellApp() : _universe(new_box<Universe>()), _logger("shell") {}

up::shell::ShellApp::~ShellApp() {
    _imguiBackend.releaseResources();

    _renderer.reset();
    _uiRenderCamera.reset();
    _swapChain.reset();
    _window.reset();

    _device.reset();
}

int up::shell::ShellApp::initialize() {
    TracyAppInfo("PotatoShell", stringLength("PotatoShell"));
    ZoneScopedN("Initialize Shell");

    zstring_view configPath = "shell.config.json";
    if (fs::fileExists(configPath)) {
        _loadConfig(configPath);
    }

    {
        char* prefPathSdl = SDL_GetPrefPath("potato", "shell");
        if (auto const rs = fs::createDirectories(prefPathSdl); rs != IOResult::Success) {
            _logger.error("Failed to create preferences folder `{}`", prefPathSdl);
        }
        _shellSettingsPath = path::join(prefPathSdl, "settings.json");
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        SDL_free(prefPathSdl);
    }

    schema::EditorSettings settings;
    if (loadShellSettings(_shellSettingsPath, settings)) {
        _logger.info("Loaded user settings: ", _shellSettingsPath);
    }

    _appActions.addAction(
        {.name = "potato.quit",
         .command = "Quit",
         .menu = "File\\Quit",
         .icon = ICON_FA_DOOR_OPEN,
         .group = "z_quit",
         .hotKey = "Alt+F4",
         .action = [this] {
             _running = false;
         }});
    _appActions.addAction(
        {.name = "potato.project.open",
         .command = "Open Project",
         .menu = "File\\Open Project",
         .icon = ICON_FA_FOLDER_OPEN,
         .group = "3_project",
         .priority = 100,
         .hotKey = "Alt+Shift+O",
         .action = [this] {
             _openProject = true;
             _closeProject = true;
         }});
    _appActions.addAction(
        {.name = "potato.project.import",
         .command = "Import Resources",
         .menu = "File\\Import Resources",
         .icon = ICON_FA_FILE_IMPORT,
         .group = "3_project",
         .priority = 120,
         .action = [this] {
             _executeRecon();
         }});
    _appActions.addAction(
        {.name = "potato.project.close",
         .command = "Close Project",
         .menu = "File\\Close Project",
         .group = "3_project",
         .priority = 1100,
         .enabled = [this] { return _project != nullptr; },
         .action =
             [this] {
                 _closeProject = true;
             }});
    _appActions.addAction(
        {.name = "potato.assets.newScene",
         .command = "New Scene",
         .menu = "File\\New\\Scene",
         .icon = ICON_FA_IMAGE,
         .group = "1_new",
         .enabled = [this]() { return _project != nullptr; },
         .action =
             [this] {
                 _createScene();
             }});
    _appActions.addAction(
        {.name = "potato.editor.about", .menu = "Help\\About", .icon = ICON_FA_QUESTION_CIRCLE, .action = [this] {
             _aboutDialog = true;
         }});
    _appActions.addAction(
        {.name = "potato.editor.logs",
         .menu = "View\\Logs",
         .icon = ICON_FA_INFO,
         .hotKey = "Alt+Shift+L",
         .checked = [this] { return _logWindow.isOpen(); },
         .action =
             [this] {
                 _logWindow.open(!_logWindow.isOpen());
             }});

    _actions.addGroup(&_appActions);

    _menu.addMenu({.menu = "File"_sv, .group = "1_file"_sv});
    _menu.addMenu({.menu = "File\\New"_sv, .group = "2_new"_sv});
    _menu.addMenu({.menu = "File\\Settings"_sv, .group = "9_settings"_sv});
    _menu.addMenu({.menu = "Edit"_sv, .group = "3_edit"_sv});
    _menu.addMenu({.menu = "View"_sv, .group = "5_view"_sv});
    _menu.addMenu({.menu = "View\\Options"_sv, .group = "5_options"_sv});
    _menu.addMenu({.menu = "View\\Panels"_sv, .group = "3_panels"_sv});
    _menu.addMenu({.menu = "Actions"_sv, .group = "7_actions"_sv});
    _menu.addMenu({.menu = "Help"_sv, .group = "9_help"_sv});

    _menu.bindActions(_actions);
    _palette.bindActions(_actions);

    _window = SDL_CreateWindow(
        "loading",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
    if (_window == nullptr) {
        _errorDialog("Could not create window");
    }
    _updateTitle();

    SDL_DisplayMode desktopMode;
    int const displayIndex = SDL_GetWindowDisplayIndex(_window.get());
    int const displayResult = SDL_GetCurrentDisplayMode(displayIndex, &desktopMode);
    if (displayResult == 0) {
        int currentWidth = 0;
        int currentHeight = 0;
        SDL_GetWindowSize(_window.get(), &currentWidth, &currentHeight);

        int const newWidth = std::max(static_cast<int>(desktopMode.w * 0.75), currentWidth);
        int const newHeight = std::max(static_cast<int>(desktopMode.h * 0.75), currentHeight);

        int const newPosX = static_cast<int>((desktopMode.w - newWidth) * 0.5);
        int const newPosY = static_cast<int>((desktopMode.h - newHeight) * 0.5);

        SDL_SetWindowSize(_window.get(), newWidth, newHeight);
        SDL_SetWindowPosition(_window.get(), newPosX, newPosY);
    }

    SDL_ShowWindow(_window.get());

    SDL_SysWMinfo wmInfo{};
    SDL_VERSION(&wmInfo.version);

    if (SDL_GetWindowWMInfo(_window.get(), &wmInfo) != SDL_TRUE) {
        _errorDialog("Could not get window info");
        return 1;
    }

    _audio = AudioEngine::create();
    _audio->registerAssetBackends(_assetLoader);

#if defined(UP_GPU_ENABLE_D3D12)
    if (_device == nullptr) {
        auto factory = CreateFactoryD3D12();
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

    _renderer = new_box<Renderer>(_device);
    _renderer->registerAssetBackends(_assetLoader);

#if UP_PLATFORM_WINDOWS
    _swapChain = _device->createSwapChain(wmInfo.info.win.window);
#endif
    if (_swapChain == nullptr) {
        _errorDialog("Failed to create swap chain");
        return 1;
    }

    _uiRenderCamera = new_box<RenderCamera>();

    // #dx12: for now kick off new frame for resource creation -- will have to build a new system for that.
    _device->beginResourceCreation();
    _imguiBackend.createResources(*_device);
    _device->endResourceCreation();

    _universe = new_box<Universe>();

    _universe->registerComponent<components::Transform>("Transform");
    _universe->registerComponent<components::Mesh>("Mesh");
    _universe->registerComponent<components::Wave>("Wave");
    _universe->registerComponent<components::Spin>("Spin");
    _universe->registerComponent<components::Ding>("Ding");
    _universe->registerComponent<components::Test>("Test");

    _editorFactories.push_back(
        AssetBrowser::createFactory(_assetLoader, _reconClient, _assetEditService, [this](UUID const& uuid) {
            _openAssetEditor(uuid);
        }));
    _editorFactories.push_back(SceneEditor::createFactory(
        *_audio,
        *_universe,
        _assetLoader,
        [this] { return _universe->components(); },
        [this](rc<Scene> scene) { _createGame(std::move(scene)); }));
    _editorFactories.push_back(MaterialEditor::createFactory(_assetLoader));

    if (!settings.project.empty()) {
        _loadProject(settings.project);
    }

    return 0;
}

bool up::shell::ShellApp::_selectAndLoadProject(zstring_view folder) {
    nfdchar_t* selectedPath = nullptr;
    auto result = NFD_OpenDialog("popr", path::normalize(folder).c_str(), &selectedPath);
    if (result == NFD_ERROR) {
        result = NFD_OpenDialog("popr", nullptr, &selectedPath);
    }
    if (result == NFD_ERROR) {
        _logger.error("NDF_OpenDialog error: {}", NFD_GetError());
        return false;
    }
    bool success = _loadProject(selectedPath);
    free(selectedPath); // NOLINT(cppcoreguidelines-no-malloc)

    if (success) {
        schema::EditorSettings settings;
        settings.project = string{_project->projectFilePath()};
        if (!saveShellSettings(_shellSettingsPath, settings)) {
            _logger.error("Failed to save shell settings to `{}`", _shellSettingsPath);
        }
    }
    return success;
}

bool up::shell::ShellApp::_loadProject(zstring_view path) {
    _logger.info("Loading project: {}", path);

    _project = Project::loadFromFile(path);
    if (_project == nullptr) {
        _errorDialog("Could not load project file");
        return false;
    }

    _projectName = string{path::filebasename(path)};
    _assetEditService.setAssetRoot(string{_project->resourceRootPath()});

    _loadManifest();

    _editors.closeAll();
    _openEditor(AssetBrowser::editorName);
    _updateTitle();

    if (!_reconClient.start(*_project)) {
        _logger.error("Failed to start recon");
    }

    return true;
}

void up::shell::ShellApp::_openEditor(zstring_view editorName) {
    for (auto const& factory : _editorFactories) {
        if (factory->editorName() == editorName) {
            auto editor = factory->createEditor();
            if (editor != nullptr) {
                _editors.open(std::move(editor));
            }
            return;
        }
    }
}

void up::shell::ShellApp::_openEditorForDocument(zstring_view editorName, zstring_view filename) {
    for (auto const& factory : _editorFactories) {
        if (factory->editorName() == editorName) {
            auto editor = factory->createEditorForDocument(filename);
            if (editor != nullptr) {
                _editors.open(std::move(editor));
            }
            return;
        }
    }
}

void up::shell::ShellApp::run() {
    auto& imguiIO = ImGui::GetIO();

    auto now = std::chrono::high_resolution_clock::now();
    _lastFrameDuration = now - now;

    int width = 0;
    int height = 0;

    constexpr double nano_to_seconds = 1.0 / 1000000000.0;

    uint64 hotKeyRevision = 0;

    while (isRunning()) {
        ZoneScopedN("Main Loop");

        imguiIO.DeltaTime = _lastFrameTime;

        if (!_actions.refresh(hotKeyRevision)) {
            ZoneScopedN("Rebuild Actions");
            _hotKeys.clear();
            _actions.build([this](ActionId id, ActionDesc const& action) {
                if (!action.hotKey.empty()) {
                    _hotKeys.addHotKey(action.hotKey, id);
                }
            });
        }

        _processEvents();

        if (_openProject && !_closeProject) {
            ZoneScopedN("Load Project");
            _openProject = false;
            if (!_selectAndLoadProject(
                    path::join(fs::currentWorkingDirectory(), "..", "..", "..", "..", "resources"))) {
                continue;
            }
        }

        if (_reconClient.hasUpdatedAssets()) {
            _loadManifest();
        }

        SDL_GetWindowSize(_window.get(), &width, &height);
        imguiIO.DisplaySize.x = static_cast<float>(width);
        imguiIO.DisplaySize.y = static_cast<float>(height);

        _imguiBackend.beginFrame();

        _displayUI();

        _imguiBackend.endFrame();

        _render();

        if (_closeProject) {
            _reconClient.stop();
            _closeProject = false;
            _editors.closeAll();
            _project = nullptr;
            _updateTitle();
        }

        auto endFrame = std::chrono::high_resolution_clock::now();
        _lastFrameDuration = endFrame - now;
        _lastFrameTime = static_cast<float>(_lastFrameDuration.count() * nano_to_seconds);
        now = endFrame;
    }

    // clean-up
    _renderer->quit();
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
    _swapChain->resizeBuffers(width, height);
    
    _logger.info("Window resized: {}x{}", width, height);
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
    ZoneScopedN("Shell Events");

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
                _imguiBackend.handleEvent(ev);
                break;
            case SDL_KEYDOWN:
                if (!_hotKeys.evaluateKey(ev.key.keysym.sym, ev.key.keysym.mod, [this](auto id) {
                        return _actions.tryInvoke(id);
                    })) {
                    _imguiBackend.handleEvent(ev);
                }
                break;
            case SDL_MOUSEBUTTONUP:
            case SDL_MOUSEMOTION:
            case SDL_MOUSEWHEEL:
            default:
                _imguiBackend.handleEvent(ev);
                break;
        }
    }
}

void up::shell::ShellApp::_render() {
    ZoneScopedN("Shell Render");

    GpuViewportDesc viewport;
    int width = 0;
    int height = 0;
    SDL_GetWindowSize(_window.get(), &width, &height);
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);

    _uiRenderCamera->setRenderTarget(std::move(_swapChain->getRenderTargetView()));

    _imguiBackend.draw(*_renderer.get(), _uiRenderCamera.get());

    _renderer->beginFrame(_swapChain.get());
    _renderer->endFrame(_swapChain.get(), 0.0f);
    _swapChain->present();
    FrameMark;
}

void up::shell::ShellApp::_displayUI() {
    ZoneScopedN("Shell UI");

    auto& imguiIO = ImGui::GetIO();

    _displayMainMenu();

    ImVec2 menuSize;
    if (ImGui::BeginMainMenuBar()) {
        menuSize = ImGui::GetWindowSize();
        ImGui::EndMainMenuBar();
    }

    _displayDocuments({0, menuSize.y, imguiIO.DisplaySize.x, imguiIO.DisplaySize.y});

    if (_aboutDialog) {
        ImGui::SetNextWindowSizeConstraints({400, 300}, {});
        ImGui::Begin("About", &_aboutDialog, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Potato editor");
        ImGui::End();
    }

    _palette.drawPalette();
}

void up::shell::ShellApp::_displayMainMenu() {
    _menu.drawMenu();

    if (ImGui::BeginMainMenuBar()) {
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
    auto const windowFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::SetNextWindowPos({rect.x, rect.y});
    ImGui::SetNextWindowSize({rect.z - rect.x, rect.w - rect.y});
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});
    ImGui::Begin("MainWindow", nullptr, windowFlags);
    ImGui::PopStyleVar(1);

    _editors.update(_actions, *_renderer, _lastFrameTime);

    _logWindow.draw();

    ImGui::End();
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

    return true;
}

void up::shell::ShellApp::_openAssetEditor(UUID const& uuid) {
    string assetPath;
    uint64 assetTypeHash = 0;
    for (auto const& record : _assetLoader.manifest()->records()) {
        if (record.uuid == uuid) {
            assetPath = path::join(_project->resourceRootPath(), record.filename);
            assetTypeHash = hash_value(record.type);
            break;
        }
    }
    if (assetPath.empty()) {
        return;
    }

    zstring_view const editor = _assetEditService.findInfoForAssetTypeHash(assetTypeHash).editor;
    if (!editor.empty()) {
        _openEditorForDocument(SceneEditor::editorName, assetPath);
    }
    else {
        if (!desktop::openInExternalEditor(assetPath)) {
            _logger.error("Failed to open application for asset: {}", assetPath);
        }
    }
}

void up::shell::ShellApp::_createScene() {
    _openEditor(SceneEditor::editorName);
}

void up::shell::ShellApp::_createGame(rc<Scene> scene) {
    _editors.open(createGameEditor(std::move(scene)));
}

void up::shell::ShellApp::_executeRecon() {
    schema::ReconImportAllMessage msg;
    msg.force = true;
    _reconClient.sendMessage(msg);

    _loadManifest();
}

void up::shell::ShellApp::_loadManifest() {
    ZoneScopedN("Load Manifest");
    string manifestPath = path::join(_project->libraryPath(), "manifest.txt");
    _logger.info("Loading manifest {}", manifestPath);
    if (auto [rs, manifestText] = fs::readText(manifestPath); rs == IOResult{}) {
        auto manifest = new_box<ResourceManifest>();
        if (!ResourceManifest::parseManifest(manifestText, *manifest)) {
            _logger.error("Failed to parse resource manifest");
        }
        string casPath = path::join(_project->libraryPath(), "cache");
        _assetLoader.bindManifest(std::move(manifest), std::move(casPath));
    }
    else {
        _logger.error("Failed to load resource manifest");
    }
}
