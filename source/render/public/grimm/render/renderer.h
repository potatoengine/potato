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
    class Buffer;
    class CommandList;
    class Device;
    class SwapChain;
} // namespace gm::gpu

namespace gm {
    class RenderContext;

    class Renderer {
    public:
        GM_RENDER_API explicit Renderer(rc<gpu::Device> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        GM_RENDER_API void beginFrame();
        GM_RENDER_API void endFrame();

        GM_RENDER_API RenderContext context();

        gpu::CommandList& commandList() const noexcept { return *_commandList; }

    private:
        void _renderMain();

        rc<gpu::Device> _device;
        box<gpu::CommandList> _commandList;
        box<gpu::Buffer> _frameDataBuffer;
        std::thread _renderThread;
        concurrency::ConcurrentQueue<RenderTask> _taskQueue;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
    };
} // namespace gm
