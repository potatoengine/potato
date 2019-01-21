// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class GpuResourceView {
    public:
        enum class Type {
            RTV,
            UAV,
            DSV
        };

        GpuResourceView() = default;
        virtual ~GpuResourceView() = default;

        GpuResourceView(GpuResourceView&&) = delete;
        GpuResourceView& operator=(GpuResourceView&&) = delete;

        virtual Type type() const = 0;
    };
} // namespace gm
