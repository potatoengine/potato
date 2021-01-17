// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/unique_resource.h"

#include "potato/render/renderer.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuDevice;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class RenderContext;
    class GpuTexture; 
} // namespace up

struct ImDrawData;
struct ImDrawList;
struct ImData;
struct ImGuiContext;
struct ImGuiIO;
using SDL_Event = union SDL_Event;

namespace up {
    class Shader;

    class ImguiBackend {
    public:
        UP_EDITOR_API ImguiBackend();
        UP_EDITOR_API ~ImguiBackend();

        UP_EDITOR_API void bindShaders(rc<Shader> vertShader, rc<Shader> pixelShader);

        UP_EDITOR_API bool createResources(GpuDevice& device);
        UP_EDITOR_API void releaseResources();

        UP_EDITOR_API bool loadFontAwesome5(Stream fontFile);
        UP_EDITOR_API bool loadFont(Stream fontFile);

        UP_EDITOR_API bool handleEvent(SDL_Event const& ev);

        UP_EDITOR_API void beginFrame();
        UP_EDITOR_API void endFrame();

        UP_EDITOR_API void draw(Renderer& renderer);

        void render(RenderContext& ctx);

        void setCaptureRelativeMouseMode(bool captured) noexcept { _captureRelativeMouseMode = captured; }
        auto isCaptureRelativeMouseMode() const noexcept -> bool { return _captureRelativeMouseMode; }

    private:
        class ImGuiRenderer : public up::IRenderable {
        public:
            ImGuiRenderer(ImguiBackend* backend) : _backend(backend) {}

            void onSchedule(up::RenderContext& ctx) override{};
            void onRender(up::RenderContext& ctx) override { _backend->render(ctx); };

        private:
            ImguiBackend* _backend;
        };

        void _initialize();
        void _ensureContext();
        void _applyStyle();

        static void _freeContext(ImGuiContext* ctx);
        static char const* _getClipboardTextContents(void* self);
        static void _setClipboardTextContents(void* self, char const* zstr);

        unique_resource<ImGuiContext*, &_freeContext> _context;
        box<GpuBuffer> _indexBuffer;
        box<GpuBuffer> _vertexBuffer;
        box<GpuBuffer> _constantBuffer;
        box<GpuPipelineState> _pipelineState;
        box<GpuResourceView> _srv;
        rc<GpuTexture> _font;
        box<GpuSampler> _sampler;
        rc<Shader> _vertShader;
        rc<Shader> _pixelShader;
        string _clipboardTextData;
        box<ImGuiRenderer> _imGuiRenderer;
        bool _captureRelativeMouseMode = false;
    };
} // namespace up
