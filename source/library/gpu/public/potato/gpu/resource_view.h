// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

namespace up::gpu {
    class GpuResourceView {
    public:
        GpuResourceView() = default;
        virtual ~GpuResourceView() = default;

        GpuResourceView(GpuResourceView&&) = delete;
        GpuResourceView& operator=(GpuResourceView&&) = delete;

        virtual ViewType type() const = 0;
    };
} // namespace up::gpu
