// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "swap_chain.h"

namespace gm {
    class D3d12SwapChain : public GpuSwapChain {
    public:
        D3d12SwapChain(com_ptr<IDXGISwapChain1> swapChain);
        virtual ~D3d12SwapChain();

        D3d12SwapChain(D3d12SwapChain&&) = delete;
        D3d12SwapChain& operator=(D3d12SwapChain&&) = delete;

        static box<GpuSwapChain> createSwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* graphicsQueue, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        box<GpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        com_ptr<IDXGISwapChain1> _swapChain;
        int _bufferIndex = 0;
    };
} // namespace gm
