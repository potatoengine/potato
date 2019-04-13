// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/unique_resource.h"
#include "potato/foundation/string.h"

namespace up::gpu {
    class GpuBuffer;
    class GpuCommandList;
    class Device;
    class PipelineState;
    class ResourceView;
    class Sampler;
} // namespace up::gpu

struct ImDrawData;
struct ImDrawList;
struct ImData;
struct ImGuiContext;
struct ImGuiIO;
using SDL_Event = union SDL_Event;

namespace up {
    class Shader;

    class DrawImgui {
    public:
        UP_RENDER_API DrawImgui();
        UP_RENDER_API ~DrawImgui();

        UP_RENDER_API void bindShaders(rc<Shader> vertShader, rc<Shader> pixelShader);

        UP_RENDER_API bool createResources(gpu::Device& device);
        UP_RENDER_API void releaseResources();

        UP_RENDER_API bool handleEvent(SDL_Event const& ev);

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void endFrame(gpu::Device& device, gpu::GpuCommandList& commandList);

    private:
        void _ensureContext();
        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext, nullptr> _context;
        box<gpu::GpuBuffer> _indexBuffer;
        box<gpu::GpuBuffer> _vertexBuffer;
        box<gpu::GpuBuffer> _constantBuffer;
        box<gpu::PipelineState> _pipelineState;
        box<gpu::ResourceView> _srv;
        box<gpu::Sampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
        string _clipboardTextData;
    };
} // namespace up
