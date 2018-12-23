// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_device.h"
#    include "com_ptr.h"
#    include "d3d12_descriptor_heap.h"
#    include "d3d12_resource.h"
#    include "d3d12_swap_chain.h"
#    include "direct3d.h"
#    include "grimm/foundation/out_ptr.h"
#    include <utility>

gm::D3d12Device::D3d12Device(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D12Device1> device, com_ptr<ID3D12CommandQueue> graphicsQueue)
    : _factory(std::move(factory)), _adaptor(std::move(adapter)), _device(std::move(device)), _graphicsQueue(std::move(graphicsQueue)) {
}

gm::D3d12Device::~D3d12Device() = default;

auto gm::D3d12Device::createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter) -> box<IGPUDevice> {
    com_ptr<ID3D12Device1> device;
    D3D12CreateDevice(adapter.get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device1), out_ptr(device));
    if (device == nullptr) {
        return nullptr;
    }

    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    com_ptr<ID3D12CommandQueue> graphicsQueue;
    device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), out_ptr(graphicsQueue));
    if (graphicsQueue == nullptr) {
        return nullptr;
    }

    return make_box<D3d12Device>(std::move(factory), std::move(adapter), std::move(device), std::move(graphicsQueue));
}

auto gm::D3d12Device::createSwapChain(void* nativeWindow) -> box<ISwapChain> {
    return D3d12SwapChain::createSwapChain(_factory.get(), _graphicsQueue.get(), nativeWindow);
}

auto gm::D3d12Device::createDescriptorHeap() -> box<IDescriptorHeap> {
    return D3d12DescriptorHeap::createDescriptorHeap(_device.get());
}

void gm::D3d12Device::createRenderTargetView(IGpuResource* renderTarget, uint64 cpuHandle) {
    auto d3d12Resource = static_cast<D3d12Resource*>(renderTarget);
    D3D12_CPU_DESCRIPTOR_HANDLE handle = {cpuHandle};
    _device->CreateRenderTargetView(d3d12Resource->get(), nullptr, handle);
}

#endif
