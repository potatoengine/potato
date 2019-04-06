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

namespace up::gpu {
    class Buffer;
    class CommandList;
    class Device;
} // namespace up::gpu

namespace up {
    class RenderContext;
    class Material;
    class Mesh;
    class Shader;
    class Texture;

    class Renderer {
    public:
        UP_RENDER_API explicit Renderer(fs::FileSystem fileSystem, rc<gpu::Device> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void endFrame(float frameTime);

        UP_RENDER_API RenderContext context();

        UP_RENDER_API rc<Mesh> loadMeshSync(zstring_view path);
        UP_RENDER_API rc<Material> loadMaterialSync(zstring_view path);
        UP_RENDER_API rc<Shader> loadShaderSync(zstring_view path);
        UP_RENDER_API rc<Texture> loadTextureSync(zstring_view path);

        gpu::Device& device() const noexcept { return *_device; }
        gpu::CommandList& commandList() const noexcept { return *_commandList; }

    private:
        void _renderMain();

        rc<gpu::Device> _device;
        box<gpu::CommandList> _commandList;
        box<gpu::Buffer> _frameDataBuffer;
        rc<Material> _debugLineMaterial;
        box<gpu::Buffer> _debugLineBuffer;
        fs::FileSystem _fileSystem;
        std::thread _renderThread;
        concurrency::ConcurrentQueue<RenderTask> _taskQueue;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
    };
} // namespace up