// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"
#include "settings.h"
#include "ui/action.h"
#include "ui/command_palette.h"
#include "ui/editor_group.h"
#include "ui/menu.h"

#include "potato/audio/audio_engine.h"
#include "potato/editor/hotkeys.h"
#include "potato/editor/imgui_backend.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/resource_loader.h"
#include "potato/spud/box.h"
#include "potato/spud/unique_resource.h"

#include <SDL.h>
#include <chrono>
#include <imgui.h>

namespace up {
    class Loader;
    class Renderer;
    class RenderCamera;
    class Node;
    class Model;
    class GpuDevice;
    class GpuSwapChain;
    class GpuTexture;
    class Scene;
    class World;
    class Camera;
    class CameraController;
    class Universe;
    class Project;
} // namespace up

namespace up::shell {
    class ShellApp {
    public:
        ShellApp();
        ~ShellApp();

        ShellApp(ShellApp const&) = delete;
        ShellApp& operator=(ShellApp const&) = delete;

        int initialize();
        void run();
        void quit();

        bool isRunning() const { return _running; }

    private:
        void _onWindowSizeChanged();
        void _onWindowClosed();

        void _updateTitle();
        void _processEvents();
        void _render();

        void _displayUI();
        void _displayMainMenu();
        void _displayDocuments(glm::vec4 rect);

        void _errorDialog(zstring_view message);

        bool _loadConfig(zstring_view path);

        void _onFileOpened(zstring_view filename);

        void _createScene();
        void _createGame(rc<Scene> scene);

        void _executeRecon();
        void _loadManifest();

        bool _selectAndLoadProject(zstring_view defaultPath);
        bool _loadProject(zstring_view path);

        bool _running = true;
        bool _openProject = false;
        bool _closeProject = false;
        bool _aboutDialog = false;
        rc<GpuDevice> _device;
        rc<GpuSwapChain> _swapChain;
        box<Loader> _loader;
        box<Renderer> _renderer;
        box<RenderCamera> _uiRenderCamera;
        box<Universe> _universe;
        box<AudioEngine> _audio;
        box<Project> _project;
        string _shellSettingsPath;
        unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
        unique_resource<SDL_Cursor*, SDL_FreeCursor> _cursor;
        ActionGroup _appActions;
        Actions _actions;
        CommandPalette _palette;
        Menu _menu;
        HotKeys _hotKeys;
        EditorGroup _editors;
        int _lastCursor = -1;
        ImguiBackend _imguiBackend;
        Logger _logger;
        float _lastFrameTime = 0.f;
        std::chrono::nanoseconds _lastFrameDuration = {};
        string _projectName;
        ResourceLoader _resourceLoader;
    }; // namespace up::shell
} // namespace up::shell
