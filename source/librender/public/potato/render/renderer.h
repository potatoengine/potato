// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "loader.h"

#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/filesystem.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class RenderContext;
    class Material;
    class Mesh;
    class Shader;
    class Texture;

    class Renderer : private Loader {
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
        rc<GpuDevice> _device;
        box<GpuCommandList> _commandList;
        box<GpuBuffer> _frameDataBuffer;
        rc<Material> _debugLineMaterial;
        box<GpuBuffer> _debugLineBuffer;
        FileSystem& _fileSystem;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
    };
} // namespace up
