// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_swap_chain.h"
#include "d3d12_platform.h"
#include "d3d12_texture.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"
#include "potato/spud/out_ptr.h"

#include <utility>

up::d3d12::SwapChainD3D12::SwapChainD3D12(IDXGISwapChainPtr swapChain, ID3DCommandQueuePtr commandQueue)
    : _swapChain(std::move(swapChain))
    , _commandQueue(std::move(commandQueue))
{}

up::d3d12::SwapChainD3D12::~SwapChainD3D12() = default;

auto up::d3d12::SwapChainD3D12::createSwapChain(IDXGIFactoryType* factory, ID3DDeviceType* device, void* nativeWindow)
    -> rc<GpuSwapChain> {

     // Describe and create the command queue.
    com_ptr<ID3D12CommandQueue> cmd;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT hr = device->CreateCommandQueue(&queueDesc, out_ptr(cmd));
    if (FAILED(hr) || cmd == nullptr) {
        return nullptr;
    }

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

    IDXGISwapChainPtr swapChain;
    hr = factory->CreateSwapChainForHwnd(cmd.get(), window, &desc, nullptr, nullptr, out_ptr(swapChain));
    if (FAILED(hr) || swapChain == nullptr) {
        cmd->Release();
        return nullptr;
    }

    factory->MakeWindowAssociation(window, DXGI_MWA_NO_ALT_ENTER);

    return new_shared<SwapChainD3D12>(std::move(swapChain), std::move(cmd));
}

void up::d3d12::SwapChainD3D12::present() {

    _swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    _bufferIndex = (_bufferIndex + 1) % 2;
}

void up::d3d12::SwapChainD3D12::resizeBuffers(int width, int height) {
    _swapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
}

auto up::d3d12::SwapChainD3D12::getBuffer(int index) -> rc<GpuTexture> {
    ID3DResourcePtr buffer;
    _swapChain->GetBuffer(index, __uuidof(ID3D12Resource), out_ptr(buffer));
    if (buffer == nullptr) {
        return nullptr;
    }
    return new_shared<TextureD3D12>(std::move(buffer));
}

int up::d3d12::SwapChainD3D12::getCurrentBufferIndex() {
    return _bufferIndex;
}
