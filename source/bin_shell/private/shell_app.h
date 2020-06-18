// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "camera.h"
#include "document.h"

#include "potato/audio/audio_engine.h"
#include "potato/audio/sound_resource.h"
#include "potato/render/draw_imgui.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/native.h"
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
    class World;
    class Scene;
    class Camera;
    class CameraController;
    class Universe;
    class SoundResource;
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

        void _processEvents();
        void _tick();
        void _render();

        void _displayUI();
        void _displayMainMenu();
        void _displayDocuments(glm::vec4 rect);

        void _errorDialog(zstring_view message);

        bool _loadConfig(zstring_view path);
        bool _loadProject(zstring_view path);
        bool _loadScene();

        bool _running = true;
        NativeFileSystem _fileSystem;
        rc<GpuDevice> _device;
        rc<GpuSwapChain> _swapChain;
        box<Loader> _loader;
        box<Renderer> _renderer;
        box<RenderCamera> _uiRenderCamera;
        box<Universe> _universe;
        rc<Scene> _scene;
        box<AudioEngine> _audio;
        box<Project> _project;
        string _editorResourcePath;
        rc<SoundResource> _ding;
        unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
        unique_resource<SDL_Cursor*, SDL_FreeCursor> _cursor;
        int _lastCursor = -1;
        DrawImgui _drawImgui;
        Logger _logger;
        vector<box<Document>> _documents;
        float _lastFrameTime = 0.f;
        std::chrono::nanoseconds _lastFrameDuration = {};
        ImGuiWindowClass _documentWindowClass;
    };
} // namespace up::shell
