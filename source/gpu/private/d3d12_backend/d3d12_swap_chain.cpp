// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d12_swap_chain.h"
#include "com_ptr.h"
#include "d3d12_resource.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/out_ptr.h"
#include <utility>
#include <spdlog/spdlog.h>

gm::D3d12SwapChain::D3d12SwapChain(com_ptr<IDXGISwapChain1> swapChain) : _swapChain(std::move(swapChain)) {}

gm::D3d12SwapChain::~D3d12SwapChain() = default;

auto gm::D3d12SwapChain::createSwapChain(IDXGIFactory2* factory, ID3D12CommandQueue* graphicsQueue, void* nativeWindow) -> box<GpuSwapChain> {
    DXGI_SWAP_CHAIN_DESC1 desc = {0};
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HWND window = static_cast<HWND>(nativeWindow);

    com_ptr<IDXGISwapChain1> swapChain;
    HRESULT hr = factory->CreateSwapChainForHwnd(graphicsQueue, window, &desc, nullptr, nullptr, out_ptr(swapChain));
    if (swapChain == nullptr) {
        SPDLOG_DEBUG("CreateSwapChain: %s", static_cast<int>(hr));
        return nullptr;
    }

    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    return make_box<D3d12SwapChain>(std::move(swapChain));
}

void gm::D3d12SwapChain::present() {
    _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    _bufferIndex = (_bufferIndex + 1) % 2;
}

void gm::D3d12SwapChain::resizeBuffers(int width, int height) {
    _swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
}

auto gm::D3d12SwapChain::getBuffer(int index) -> box<GpuResource> {
    com_ptr<ID3D12Resource> buffer;
    _swapChain->GetBuffer(index, __uuidof(ID3D12Resource), out_ptr(buffer));
    if (buffer == nullptr) {
        return nullptr;
    }
    return make_box<D3d12Resource>(std::move(buffer));
}

int gm::D3d12SwapChain::getCurrentBufferIndex() {
    return _bufferIndex;
}
