// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace gm::gpu {
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
    class RenderContext {
    public:
        double frameTime = 0;
        gpu::CommandList& commandList;
        gpu::Device& device;
    };
} // namespace gm
