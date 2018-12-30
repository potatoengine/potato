// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"

namespace gm {
    class IGpuResource;

    class ISwapChain {
    public:
        ISwapChain() = default;
        virtual ~ISwapChain() = default;

        ISwapChain(ISwapChain&&) = delete;
        ISwapChain& operator=(ISwapChain&&) = delete;

        virtual void present() = 0;
        virtual void resizeBuffers(int width, int height) = 0;
        virtual box<IGpuResource> getBuffer(int index) = 0;
        virtual int getCurrentBufferIndex() = 0;
    };
} // namespace gm
