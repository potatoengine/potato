// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "render_task.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/zstring_view.h"
#include "potato/filesystem/filesystem.h"
#include "potato/concurrency/concurrent_queue.h"
#include <thread>
#include <atomic>

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class RenderContext;
    class Material;
    class Mesh;
    class Shader;
    class Texture;

    class Renderer {
    public:
        UP_RENDER_API explicit Renderer(FileSystem& fileSystem, rc<GpuDevice> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void flushDebugDraw(float frameTime);
        UP_RENDER_API void endFrame(float frameTime);

        UP_RENDER_API RenderContext context();

        UP_RENDER_API rc<Mesh> loadMeshSync(zstring_view path);
        UP_RENDER_API rc<Material> loadMaterialSync(zstring_view path);
        UP_RENDER_API rc<Shader> loadShaderSync(zstring_view path);
        UP_RENDER_API rc<Texture> loadTextureSync(zstring_view path);

        GpuDevice& device() const noexcept { return *_device; }
        GpuCommandList& commandList() const noexcept { return *_commandList; }

    private:
        void _renderMain();

        rc<GpuDevice> _device;
        box<GpuCommandList> _commandList;
        box<GpuBuffer> _frameDataBuffer;
        rc<Material> _debugLineMaterial;
        box<GpuBuffer> _debugLineBuffer;
        FileSystem& _fileSystem;
        std::thread _renderThread;
        ConcurrentQueue<RenderTask> _taskQueue;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
    };
} // namespace up
