// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class IGpuResource {
    public:
        IGpuResource() = default;
        virtual ~IGpuResource();

        IGpuResource(IGpuResource&&) = delete;
        IGpuResource& operator=(IGpuResource&&) = delete;
    };
} // namespace gm
