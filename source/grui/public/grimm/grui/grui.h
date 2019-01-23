// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/blob.h"

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
struct ImGuiIO;

namespace gm::gui {
    class DrawImgui {
    public:
        GM_GRUI_API DrawImgui();
        GM_GRUI_API ~DrawImgui();

        GM_GRUI_API bool createResources(gpu::Device& device, ImGuiIO const& imguiIO, blob vertShader, blob pixelShader);
        GM_GRUI_API void releaseResources();

        GM_GRUI_API void draw(ImDrawData const& data, gpu::CommandList& commandList);

    private:
        box<gpu::Buffer> _indexBuffer;
        box<gpu::Buffer> _vertexBuffer;
        box<gpu::Buffer> _constantBuffer;
        box<gpu::PipelineState> _pipelineState;
        box<gpu::ResourceView> _srv;
        box<gpu::Sampler> _sampler;
    };
} // namespace gm::gui
