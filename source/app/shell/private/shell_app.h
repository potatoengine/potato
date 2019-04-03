// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/render/draw_imgui.h"

#include <SDL.h>

namespace up {
    class ShellApp;
    class Renderer;
    class RenderCamera;
    class Node;
} // namespace up

namespace up::gpu {
    class Device;
    class SwapChain;
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

private:
    bool _running = true;
    fs::FileSystem _fileSystem;
    rc<gpu::Device> _device;
    rc<gpu::SwapChain> _swapChain;
    box<Renderer> _renderer;
    box<RenderCamera> _camera;
    box<Node> _root;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
    DrawImgui _drawImgui;
};
