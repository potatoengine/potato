// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "potato/foundation/box.h"
#include "potato/foundation/unique_resource.h"
#include "potato/filesystem/native.h"
#include "potato/render/draw_imgui.h"
#include "potato/logger/logger.h"
#include "camera.h"

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
    void _drawUI();
    void _drawGrid();

    void _errorDialog(zstring_view message);

    bool _loadConfig(zstring_view path);

private:
    struct InputState;

    bool _running = true;
    bool _paused = true;
    bool _grid = true;
    NativeFileSystem _fileSystem;
    rc<GpuDevice> _device;
    rc<GpuSwapChain> _swapChain;
    box<Renderer> _renderer;
    box<RenderCamera> _renderCamera;
    box<Scene> _scene;
    string _resourceDir;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    DrawImgui _drawImgui;
    Logger _logger;
    Camera _camera;
    box<CameraController> _cameraController;
    box<InputState> _inputState;
    float _lastFrameTime = 0.f;
    std::chrono::nanoseconds _lastFrameDuration;
};
