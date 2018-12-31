// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class IPipelineState {
    public:
        IPipelineState() = default;
        virtual ~IPipelineState() = default;

        IPipelineState(IPipelineState&&) = delete;
        IPipelineState& operator=(IPipelineState&&) = delete;
    };
} // namespace gm
