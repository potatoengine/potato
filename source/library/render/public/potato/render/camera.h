// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/gpu/swap_chain.h"
#include <glm/mat4x4.hpp>

namespace up::gpu {
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
    class GpuResourceView;
    class GpuTexture;
} // namespace up::gpu

namespace up {
    class RenderContext;

    class RenderCamera {
    public:
        UP_RENDER_API explicit RenderCamera(rc<gpu::GpuSwapChain> swapChain = nullptr);
        UP_RENDER_API ~RenderCamera();

        UP_RENDER_API void resetSwapChain(rc<gpu::GpuSwapChain> swapChain);

        UP_RENDER_API void beginFrame(RenderContext& ctx, glm::mat4x4 cameraTransform);
        UP_RENDER_API void endFrame(RenderContext& ctx);

    private:
        rc<gpu::GpuSwapChain> _swapChain;
        box<gpu::GpuBuffer> _cameraDataBuffer;
        box<gpu::GpuTexture> _backBuffer;
        box<gpu::GpuTexture> _depthStencilBuffer;
        box<gpu::GpuResourceView> _rtv;
        box<gpu::GpuResourceView> _dsv;
    };
} // namespace up
