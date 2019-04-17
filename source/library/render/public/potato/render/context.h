// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace up {
    class GpuCommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class RenderContext {
    public:
        double frameTime = 0;
        GpuCommandList& commandList;
        GpuDevice& device;
    };
} // namespace up
