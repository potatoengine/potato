// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm {
    class ISwapChain {
    public:
        ISwapChain() = default;
        virtual ~ISwapChain();

        ISwapChain(ISwapChain&&) = delete;
        ISwapChain& operator=(ISwapChain&) = delete;

        virtual void present() = 0;
        virtual void resizeBuffers(int width, int height) = 0;
    };
} // namespace gm
