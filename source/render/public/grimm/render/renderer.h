// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "render_task.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/concurrency/concurrent_queue.h"
#include <thread>
#include <atomic>

namespace gm::gpu {
    class Device;
    class SwapChain;
} // namespace gm::gpu

namespace gm {
    class Renderer {
    public:
        GM_RENDER_API explicit Renderer(rc<gpu::Device> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

    private:
        void _renderMain();

        rc<gpu::Device> _device;
        std::thread _renderThread;
        concurrency::ConcurrentQueue<RenderTask> _taskQueue;
    };
} // namespace gm
