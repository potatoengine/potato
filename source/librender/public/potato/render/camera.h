// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/render/gpu_swap_chain.h"
#include <glm/mat4x4.hpp>

namespace up {
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
    class GpuResourceView;
    class GpuTexture;
} // namespace up

namespace up {
    class RenderContext;

    class RenderCamera {
    public:
        UP_RENDER_API explicit RenderCamera(rc<GpuSwapChain> swapChain = nullptr);
        UP_RENDER_API ~RenderCamera();

        UP_RENDER_API void resetSwapChain(rc<GpuSwapChain> swapChain);

        UP_RENDER_API void beginFrame(RenderContext& ctx, glm::vec3 cameraPosition, glm::mat4x4 cameraTransform);
        UP_RENDER_API void endFrame(RenderContext& ctx);

    private:
        rc<GpuSwapChain> _swapChain;
        box<GpuBuffer> _cameraDataBuffer;
        box<GpuTexture> _backBuffer;
        box<GpuTexture> _depthStencilBuffer;
        box<GpuResourceView> _rtv;
        box<GpuResourceView> _dsv;
    };
} // namespace up
