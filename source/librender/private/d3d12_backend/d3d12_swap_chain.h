// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_swap_chain.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {
    class SwapChainD3D12 : public GpuSwapChain {
    public:
        SwapChainD3D12(IDXGISwapChainPtr swapChain, ID3DCommandQueuePtr commandQueue);
        virtual ~SwapChainD3D12();

        SwapChainD3D12(SwapChainD3D12&&) = delete;
        SwapChainD3D12& operator=(SwapChainD3D12&&) = delete;

        static rc<GpuSwapChain> createSwapChain(IDXGIFactory2* factory, ID3D12Device* device, void* nativeWindow);

        void present() override;
        void resizeBuffers(int width, int height) override;
        rc<GpuTexture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;

    private:
        IDXGISwapChainPtr _swapChain;
        ID3DCommandQueuePtr _commandQueue;
        int _bufferIndex = 0;
    };
} // namespace up::d3d12
