// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/descriptor_heap.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"

#include <SDL.h>

namespace gm {
    class ShellApp;
}

class gm::ShellApp {
public:
    ShellApp() = default;
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
    gm::box<gm::GpuDevice> _device;
    gm::box<gm::GpuSwapChain> _swapChain;
    gm::box<gm::GpuDescriptorHeap> _rtvHeap;
    gm::box<gm::GpuCommandList> _commandList;
    gm::unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
};
