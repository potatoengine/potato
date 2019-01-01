// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class GpuResource {
    public:
        GpuResource() = default;
        virtual ~GpuResource() = default;

        GpuResource(GpuResource&&) = delete;
        GpuResource& operator=(GpuResource&&) = delete;
    };
} // namespace gm
