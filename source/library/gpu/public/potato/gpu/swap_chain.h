// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"

namespace up::gpu {
    class GpuTexture;

    class GpuSwapChain : public shared<GpuSwapChain> {
    public:
        GpuSwapChain() = default;
        virtual ~GpuSwapChain() = default;

        GpuSwapChain(GpuSwapChain&&) = delete;
        GpuSwapChain& operator=(GpuSwapChain&&) = delete;

        virtual void present() = 0;
        virtual void resizeBuffers(int width, int height) = 0;
        virtual box<GpuTexture> getBuffer(int index) = 0;
        virtual int getCurrentBufferIndex() = 0;
    };
} // namespace up::gpu
