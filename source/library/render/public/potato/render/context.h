// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace up::gpu {
    class GpuCommandList;
    class Device;
} // namespace up::gpu

namespace up {
    class RenderContext {
    public:
        double frameTime = 0;
        gpu::GpuCommandList& commandList;
        gpu::Device& device;
    };
} // namespace up
