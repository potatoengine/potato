// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/unique_resource.h"
#include "potato/foundation/string.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
} // namespace up

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

        UP_RENDER_API bool createResources(GpuDevice& device);
        UP_RENDER_API void releaseResources();

        UP_RENDER_API bool handleEvent(SDL_Event const& ev);

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void endFrame(GpuDevice& device, GpuCommandList& commandList);

    private:
        void _ensureContext();
        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext, nullptr> _context;
        box<GpuBuffer> _indexBuffer;
        box<GpuBuffer> _vertexBuffer;
        box<GpuBuffer> _constantBuffer;
        box<GpuPipelineState> _pipelineState;
        box<GpuResourceView> _srv;
        box<GpuSampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
        string _clipboardTextData;
    };
} // namespace up
