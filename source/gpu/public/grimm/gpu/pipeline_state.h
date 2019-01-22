// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/blob.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/types.h"

namespace gm {
    struct GpuPipelineStateDesc {
        blob vertShader;
        blob pixelShader;
        span<InputLayoutElement const> inputLayout;
    };

    class GpuPipelineState {
    public:
        GpuPipelineState() = default;
        virtual ~GpuPipelineState() = default;

        GpuPipelineState(GpuPipelineState&&) = delete;
        GpuPipelineState& operator=(GpuPipelineState&&) = delete;
    };
} // namespace gm
