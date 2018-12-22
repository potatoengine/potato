// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_device.h"
#    include "com_ptr.h"
#    include "d3d12_swap_chain.h"
#    include "direct3d.h"
#    include "grimm/foundation/out_ptr.h"
#    include <utility>

gm::D3d12Device::D3d12Device(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adaptor, com_ptr<ID3D12Device1> device, com_ptr<ID3D12CommandQueue> graphicsQueue)
    : _factory(std::move(factory)), _adaptor(std::move(adaptor)), _device(std::move(device)), _graphicsQueue(std::move(graphicsQueue)) {
}

gm::D3d12Device::~D3d12Device() = default;

auto gm::D3d12Device::createSwapChain(void* nativeWindow) -> box<ISwapChain> {
    DXGI_SWAP_CHAIN_DESC1 desc = {0};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UINT;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_BACK_BUFFER | DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    desc.Scaling = DXGI_SCALING_ASPECT_RATIO_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HWND window = static_cast<HWND>(nativeWindow);

    _factory->MakeWindowAssociation(window, DXGI_MWA_NO_WINDOW_CHANGES);

    com_ptr<IDXGISwapChain1> swapChain;
    _factory->CreateSwapChainForHwnd(_graphicsQueue.get(), window, &desc, nullptr, nullptr, out_ptr(swapChain));
    return make_box<D3d12SwapChain>(std::move(swapChain));
}

#endif
