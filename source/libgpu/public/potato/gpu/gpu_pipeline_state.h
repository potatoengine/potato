// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "gpu_common.h"

namespace up {
    class GpuPipelineState {
    public:
        GpuPipelineState() = default;
        virtual ~GpuPipelineState() = default;

        GpuPipelineState(GpuPipelineState&&) = delete;
        GpuPipelineState& operator=(GpuPipelineState&&) = delete;
    };
} // namespace up
