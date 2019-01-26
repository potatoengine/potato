// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include <thread>
#include <atomic>

namespace gm::gpu {
    class Device;
    class SwapChain;
} // namespace gm::gpu

namespace gm {
    class Renderer {
    public:
        explicit GM_RENDER_API Renderer(rc<gpu::Device> device);
        GM_RENDER_API ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        GM_RENDER_API void bindSwapChain(rc<gpu::SwapChain> swapChain);

    private:
        struct Backend;

        box<Backend> _backend;
    };
} // namespace gm
