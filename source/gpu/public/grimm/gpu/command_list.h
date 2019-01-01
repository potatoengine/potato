// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"
#include "grimm/math/packed.h"
#include "transition.h"

namespace gm {
    class GpuResource;
    class GpuPipelineState;

    class GpuCommandList {
    public:
        GpuCommandList() = default;
        virtual ~GpuCommandList() = default;

        GpuCommandList(GpuCommandList&&) = delete;
        GpuCommandList& operator=(GpuCommandList&&) = delete;

        virtual void clearRenderTarget(uint64 handle, PackedVector4f color) = 0;
        virtual void resourceBarrier(GpuResource* resource, GpuResourceState from, GpuResourceState to) = 0;

        virtual void reset(GpuPipelineState* pipelineState = nullptr) = 0;
    };
} // namespace gm
