// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "potato/foundation/box.h"
#include "potato/foundation/unique_resource.h"
#include "potato/filesystem/native.h"
#include "potato/render/draw_imgui.h"
#include "potato/logger/logger.h"

#include <SDL.h>

namespace up {
    class ShellApp;
    class Renderer;
    class RenderCamera;
    class Node;
    class Model;
    class GpuDevice;
    class GpuSwapChain;
    class World;
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

    void _errorDialog(zstring_view message);

    bool _loadConfig(zstring_view path);

private:
    bool _running = true;
    NativeFileSystem _fileSystem;
    rc<GpuDevice> _device;
    rc<GpuSwapChain> _swapChain;
    box<Renderer> _renderer;
    box<RenderCamera> _camera;
    rc<Model> _cube;
    box<Node> _root;
    box<World> _world;
    string _resourceDir;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    DrawImgui _drawImgui;
    Logger _logger;
};
