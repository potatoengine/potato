// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"
#include "grimm/gpu/swap_chain.h"
#include <glm/mat4x4.hpp>

namespace up::gpu {
    class Buffer;
    class CommandList;
    class Device;
    class ResourceView;
    class Texture;
} // namespace up::gpu

namespace up {
    class RenderContext;

    class RenderCamera {
    public:
        UP_RENDER_API explicit RenderCamera(rc<gpu::SwapChain> swapChain = nullptr);
        UP_RENDER_API ~RenderCamera();

        UP_RENDER_API void resetSwapChain(rc<gpu::SwapChain> swapChain);

        UP_RENDER_API void beginFrame(RenderContext& ctx, glm::mat4x4 cameraTransform);
        UP_RENDER_API void endFrame(RenderContext& ctx);

    private:
        rc<gpu::SwapChain> _swapChain;
        box<gpu::Buffer> _cameraDataBuffer;
        box<gpu::Texture> _backBuffer;
        box<gpu::Texture> _depthStencilBuffer;
        box<gpu::ResourceView> _rtv;
        box<gpu::ResourceView> _dsv;
    };
} // namespace up
