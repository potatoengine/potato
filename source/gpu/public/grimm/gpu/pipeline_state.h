// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

namespace gm::gpu {
    class PipelineState {
    public:
        PipelineState() = default;
        virtual ~PipelineState() = default;

        PipelineState(PipelineState&&) = delete;
        PipelineState& operator=(PipelineState&&) = delete;
    };
} // namespace gm::gpu
