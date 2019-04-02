// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/rc.h"

namespace gm::gpu {
    class Texture;

    class SwapChain : public shared<SwapChain> {
    public:
        SwapChain() = default;
        virtual ~SwapChain() = default;

        SwapChain(SwapChain&&) = delete;
        SwapChain& operator=(SwapChain&&) = delete;

        virtual void present() = 0;
        virtual void resizeBuffers(int width, int height) = 0;
        virtual box<Texture> getBuffer(int index) = 0;
        virtual int getCurrentBufferIndex() = 0;
    };
} // namespace gm::gpu
