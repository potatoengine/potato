// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

namespace up::gpu {
    class GpuPipelineState {
    public:
        GpuPipelineState() = default;
        virtual ~GpuPipelineState() = default;

        GpuPipelineState(GpuPipelineState&&) = delete;
        GpuPipelineState& operator=(GpuPipelineState&&) = delete;
    };
} // namespace up::gpu
