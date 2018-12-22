// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_swap_chain.h"
#    include "com_ptr.h"
#    include "direct3d.h"
#    include <utility>

gm::D3d12SwapChain::D3d12SwapChain(com_ptr<IDXGISwapChain1> swapChain) : _swapChain(std::move(swapChain)) {}

gm::D3d12SwapChain::~D3d12SwapChain() = default;

void gm::D3d12SwapChain::present() {
    _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}

#endif
