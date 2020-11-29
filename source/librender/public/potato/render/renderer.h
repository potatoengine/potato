// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "loader.h"

#include "potato/runtime/concurrent_queue.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/vector.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class RenderContext;
    class Material;
    class Mesh;
    class Shader;
    class Texture;
    class ResourceLoader;
    class GpuRenderable;

    // client-side interface of render object.  
    class IRenderable {
    public: 
        virtual void onSchedule(RenderContext& context) = 0;
        virtual void onRender(RenderContext& context) = 0; 
    };

    class Renderer {
    public:
        UP_RENDER_API Renderer(Loader& loader, rc<GpuDevice> device);
        virtual ~Renderer();

        Renderer(Renderer const&) = delete;
        Renderer& operator=(Renderer const&) = delete;

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void flushDebugDraw(float frameTime);
        UP_RENDER_API void endFrame(float frameTime);

        UP_RENDER_API GpuRenderable* createRendarable(IRenderable* pInterface);

        GpuDevice& device() const noexcept { return *_device; }
    
    private:
        rc<GpuDevice> _device;
        box<GpuBuffer> _frameDataBuffer;
        rc<Material> _debugLineMaterial;
        box<GpuBuffer> _debugLineBuffer;
        Loader& _loader;
        uint32 _frameCounter = 0;
        uint64 _startTimestamp = 0;
        double _frameTimestamp = 0;
        vector<box<GpuRenderable>> _rendarables;
    };

    class DefaultLoader : public Loader {
    public:
        UP_RENDER_API DefaultLoader(ResourceLoader& resourceLoader, rc<GpuDevice> device);
        ~DefaultLoader() override;

        rc<Mesh> loadMeshSync(zstring_view path) override;
        rc<Material> loadMaterialSync(zstring_view path) override;
        rc<Shader> loadShaderSync(zstring_view path, string_view logicalName) override;
        rc<Texture> loadTextureSync(zstring_view path) override;

    private:
        ResourceLoader& _resourceLoader;
        rc<GpuDevice> _device;
    };
} // namespace up
