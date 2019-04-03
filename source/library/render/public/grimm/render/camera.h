// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/gpu/swap_chain.h"
#include <glm/mat4x4.hpp>

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class Device;
    class ResourceView;
    class Texture;
} // namespace gm::gpu

namespace gm {
    class RenderContext;

    class RenderCamera {
    public:
        GM_RENDER_API explicit RenderCamera(rc<gpu::SwapChain> swapChain = nullptr);
        GM_RENDER_API ~RenderCamera();

        GM_RENDER_API void resetSwapChain(rc<gpu::SwapChain> swapChain);

        GM_RENDER_API void beginFrame(RenderContext& ctx, glm::mat4x4 cameraTransform);
        GM_RENDER_API void endFrame(RenderContext& ctx);

    private:
        rc<gpu::SwapChain> _swapChain;
        box<gpu::Buffer> _cameraDataBuffer;
        box<gpu::Texture> _backBuffer;
        box<gpu::Texture> _depthStencilBuffer;
        box<gpu::ResourceView> _rtv;
        box<gpu::ResourceView> _dsv;
    };
} // namespace gm
