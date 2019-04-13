// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/box.h"
#include "potato/gpu/swap_chain.h"

namespace up::gpu::d3d11 {
    class SwapChainD3D11 : public GpuSwapChain {
    public:
        SwapChainD3D11(com_ptr<IDXGISwapChain1> swapChain);
        virtual ~SwapChainD3D11();

        SwapChainD3D11(SwapChainD3D11&&) = delete;
        SwapChainD3D11& operator=(SwapChainD3D11&&) = delete;

        static rc<GpuSwapChain> createSwapChain(IDXGIFactory2* factory, ID3D11Device* device, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        box<Texture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        com_ptr<IDXGISwapChain1> _swapChain;
        int _bufferIndex = 0;
    };
} // namespace up::gpu::d3d11
