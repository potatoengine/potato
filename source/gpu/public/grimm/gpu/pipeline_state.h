// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/blob.h"
#include "grimm/foundation/types.h"

namespace gm {
    struct GpuPipelineStateDesc {
        blob vertShader;
        blob pixelShader;
    };

    class GpuPipelineState {
    public:
        GpuPipelineState() = default;
        virtual ~GpuPipelineState() = default;

        GpuPipelineState(GpuPipelineState&&) = delete;
        GpuPipelineState& operator=(GpuPipelineState&&) = delete;
    };
} // namespace gm
