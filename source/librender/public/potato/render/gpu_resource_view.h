// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "gpu_common.h"

namespace up {
    class GpuResourceView {
    public:
        GpuResourceView() = default;
        virtual ~GpuResourceView() = default;

        GpuResourceView(GpuResourceView&&) = delete;
        GpuResourceView& operator=(GpuResourceView&&) = delete;

        virtual GpuViewType type() const = 0;
    };
} // namespace up
