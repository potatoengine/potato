// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "potato/spud/box.h"
#include "potato/spud/unique_resource.h"
#include "potato/runtime/native.h"
#include "potato/render/draw_imgui.h"
#include "potato/runtime/logger.h"
#include "camera.h"
#include "potato/shell/document.h"

#include <SDL.h>
#include <chrono>

namespace up {
    class ShellApp;
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
} // namespace up

class up::ShellApp {
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

private:
    struct InputState;

    bool _running = true;
    bool _showInspector = true;
    NativeFileSystem _fileSystem;
    rc<GpuDevice> _device;
    rc<GpuSwapChain> _swapChain;
    box<Renderer> _renderer;
    box<RenderCamera> _uiRenderCamera;
    box<Scene> _scene;
    string _resourceDir;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    unique_resource<SDL_Cursor*, SDL_FreeCursor> _cursor;
    int _lastCursor = -1;
    DrawImgui _drawImgui;
    Logger _logger;
    vector<box<shell::Document>> _documents;
    float _lastFrameTime = 0.f;
    float _inspectorWidth = 300.f;
    std::chrono::nanoseconds _lastFrameDuration = {};
};
