// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/gpu/pipeline_state.h"

namespace gm {
    class VknPipelineState : GpuPipelineState {
    public:
        VknPipelineState() = default;
        virtual ~VknPipelineState() = default;
    };
} // namespace gm
