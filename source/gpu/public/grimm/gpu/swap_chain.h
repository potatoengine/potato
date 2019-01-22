// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"

namespace gm::gpu {
    class GpuResource;

    class GpuSwapChain {
    public:
        GpuSwapChain() = default;
        virtual ~GpuSwapChain() = default;

        GpuSwapChain(GpuSwapChain&&) = delete;
        GpuSwapChain& operator=(GpuSwapChain&&) = delete;

        virtual void present() = 0;
        virtual void resizeBuffers(int width, int height) = 0;
        virtual box<GpuResource> getBuffer(int index) = 0;
        virtual int getCurrentBufferIndex() = 0;
    };
} // namespace gm::gpu
