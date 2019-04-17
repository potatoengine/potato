// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace up {
    class GpuSampler {
    public:
        GpuSampler() = default;
        virtual ~GpuSampler() = default;

        GpuSampler(GpuSampler&&) = delete;
        GpuSampler& operator=(GpuSampler&&) = delete;
    };
} // namespace up
