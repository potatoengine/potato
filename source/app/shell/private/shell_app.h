// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "potato/foundation/box.h"
#include "potato/foundation/unique_resource.h"
#include "potato/filesystem/filesystem.h"
#include "potato/render/draw_imgui.h"
#include "potato/logger/logger.h"

#include <SDL.h>

namespace up {
    class ShellApp;
    class Renderer;
    class RenderCamera;
    class Node;
} // namespace up

namespace up::gpu {
    class GpuDevice;
    class GpuSwapChain;
} // namespace up::gpu

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
    void onWindowSizeChanged();
    void onWindowClosed();

    void _errorDialog(zstring_view message);

private:
    bool _running = true;
    FileSystem _fileSystem;
    rc<gpu::GpuDevice> _device;
    rc<gpu::GpuSwapChain> _swapChain;
    box<Renderer> _renderer;
    box<RenderCamera> _camera;
    box<Node> _root;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    DrawImgui _drawImgui;
    Logger _logger;
};
