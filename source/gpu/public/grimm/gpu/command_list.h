// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/types.h"
#include "transition.h"

namespace gm {
    class IGpuResource;

    class ICommandList {
    public:
        ICommandList() = default;
        virtual ~ICommandList() = default;

        ICommandList(ICommandList&&) = delete;
        ICommandList& operator=(ICommandList&&) = delete;

        virtual void clearRenderTarget(uint64 handle) = 0;
        virtual void resourceBarrier(IGpuResource* resource, GpuResourceState from, GpuResourceState to) = 0;

        virtual void reset() = 0;
    };
} // namespace gm
