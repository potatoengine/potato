// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/string.h"

namespace up::gpu {
    class Buffer;
    class CommandList;
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
        UP_RENDER_API void endFrame(gpu::Device& device, gpu::CommandList& commandList);

    private:
        void _ensureContext();
        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext, nullptr> _context;
        box<gpu::Buffer> _indexBuffer;
        box<gpu::Buffer> _vertexBuffer;
        box<gpu::Buffer> _constantBuffer;
        box<gpu::PipelineState> _pipelineState;
        box<gpu::ResourceView> _srv;
        box<gpu::Sampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
        string _clipboardTextData;
    };
} // namespace up
