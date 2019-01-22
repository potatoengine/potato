// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "grimm/foundation/box.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/swap_chain.h"
#include "grimm/gpu/pipeline_state.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/resource_view.h"

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
    fs::FileSystem _fileSystem;
    box<gpu::GpuDevice> _device;
    box<gpu::GpuSwapChain> _swapChain;
    box<gpu::Buffer> _vbo;
    box<gpu::GpuResourceView> _rtv;
    box<gpu::GpuResourceView> _srv;
    box<gpu::GpuCommandList> _commandList;
    box<gpu::GpuPipelineState> _pipelineState;
    unique_resource<SDL_Window*, SDL_DestroyWindow> _window;
};
