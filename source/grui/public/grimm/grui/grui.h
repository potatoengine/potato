// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/blob.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/foundation/string_blob.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class Device;
    class PipelineState;
    class ResourceView;
    class Sampler;
} // namespace gm::gpu

struct ImDrawData;
struct ImDrawList;
struct ImData;
struct ImGuiContext;
struct ImGuiIO;
using SDL_Event = union SDL_Event;

namespace gm::gui {
    class DrawImgui {
    public:
        GM_GRUI_API DrawImgui();
        GM_GRUI_API ~DrawImgui();

        GM_GRUI_API void bindShaders(blob vertShader, blob pixelShader);

        GM_GRUI_API bool createResources(gpu::Device& device);
        GM_GRUI_API void releaseResources();

        GM_GRUI_API bool handleEvent(SDL_Event const& ev);

        GM_GRUI_API void beginFrame();
        GM_GRUI_API void endFrame(gpu::Device& device, gpu::CommandList& commandList);

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
        blob _vertShaderBlob;
        blob _pixelShaderBlob;
        string _clipboardTextData;
    };
} // namespace gm::gui
