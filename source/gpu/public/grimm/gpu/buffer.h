// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"
#include "grimm/foundation/span.h"

namespace gm {
    class GpuBuffer {
    public:
        enum class Type {
            Constant,
            Index,
            Vertex,
        };

        GpuBuffer() = default;
        virtual ~GpuBuffer() = default;

        GpuBuffer(GpuBuffer&&) = delete;
        GpuBuffer& operator=(GpuBuffer&&) = delete;

        virtual Type type() const noexcept = 0;
        virtual uint64 size() const noexcept = 0;
    };
} // namespace gm
