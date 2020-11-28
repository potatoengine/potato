// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/concurrent_queue.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuDevice;
    class RenderContext;
    class Material;
    class Mesh;
    class Shader;
    class Texture;
    class ResourceLoader;

    class Renderer {
    public:
        UP_RENDER_API Renderer(rc<GpuDevice> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void flushDebugDraw(float frameTime);
        UP_RENDER_API void endFrame(float frameTime);

        UP_RENDER_API RenderContext context();

        GpuDevice& device() const noexcept { return *_device; }
        GpuCommandList& commandList() const noexcept { return *_commandList; }

        UP_RENDER_API void registerResourceBackends(ResourceLoader& resourceLoader);

    private:
        rc<GpuDevice> _device;
        box<GpuCommandList> _commandList;
        box<GpuBuffer> _frameDataBuffer;
        box<GpuPipelineState> _debugState;
        box<GpuBuffer> _debugBuffer;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
    };
} // namespace up
