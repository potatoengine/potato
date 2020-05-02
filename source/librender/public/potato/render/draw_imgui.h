// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/string.h"
#include "potato/runtime/stream.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class RenderContext;
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

        UP_RENDER_API bool loadFontAwesome5(Stream fontFile);
        UP_RENDER_API bool loadFont(Stream fontFile);

        UP_RENDER_API bool handleEvent(SDL_Event const& ev);

        UP_RENDER_API void beginFrame();
        UP_RENDER_API void endFrame();

        UP_RENDER_API void render(RenderContext& ctx);

        void setCaptureRelativeMouseMode(bool captured) noexcept { _captureRelativeMouseMode = captured; }
        auto isCaptureRelativeMouseMode() noexcept -> bool { return _captureRelativeMouseMode; }

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
        bool _captureRelativeMouseMode = false;
    };
} // namespace up

namespace ImGui {
    UP_RENDER_API void SetCaptureRelativeMouseMode(bool captured);
    UP_RENDER_API auto IsCaptureRelativeMouseMode() -> bool;
} // namespace ImGui
