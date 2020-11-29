// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "potato/spud/int_types.h"

namespace up {

    struct FrameData {
        uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };

    class GpuCommandList;
    class GpuDevice;

    class RenderContext {
    public:
        double frameTime = 0;
        GpuCommandList& commandList;
        GpuDevice& device;
    };
} // namespace up
