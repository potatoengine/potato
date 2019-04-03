// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/swap_chain.h"

namespace up::gpu::d3d11 {
    class SwapChainD3D11 : public SwapChain {
    public:
        SwapChainD3D11(com_ptr<IDXGISwapChain1> swapChain);
        virtual ~SwapChainD3D11();

        SwapChainD3D11(SwapChainD3D11&&) = delete;
        SwapChainD3D11& operator=(SwapChainD3D11&&) = delete;

        static rc<SwapChain> createSwapChain(IDXGIFactory2* factory, ID3D11Device* device, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        box<Texture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        com_ptr<IDXGISwapChain1> _swapChain;
        int _bufferIndex = 0;
    };
} // namespace up::gpu::d3d11
