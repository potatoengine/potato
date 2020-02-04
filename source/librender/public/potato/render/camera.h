// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
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
        UP_RENDER_API RenderCamera();
        UP_RENDER_API ~RenderCamera();

        UP_RENDER_API void resetBackBuffer(rc<GpuTexture> texture);

        UP_RENDER_API void beginFrame(RenderContext& ctx, glm::vec3 cameraPosition, glm::mat4x4 cameraTransform);
        UP_RENDER_API void endFrame(RenderContext& ctx);

    private:
        box<GpuBuffer> _cameraDataBuffer;
        rc<GpuTexture> _backBuffer;
        rc<GpuTexture> _depthStencilBuffer;
        box<GpuResourceView> _rtv;
        box<GpuResourceView> _dsv;
    };
} // namespace up
