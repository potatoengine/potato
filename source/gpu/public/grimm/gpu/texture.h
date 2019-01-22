// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm::gpu {
    class GpuTexture {
    public:
        GpuTexture() = default;
        virtual ~GpuTexture() = default;

        GpuTexture(GpuTexture&&) = delete;
        GpuTexture& operator=(GpuTexture&&) = delete;
    };
} // namespace gm::gpu
