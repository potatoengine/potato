// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_device.h"
#    include "com_ptr.h"
#    include <utility>

gm::D3d12Device::D3d12Device(com_ptr<IDXGIFactory1> factory, com_ptr<IDXGIAdapter1> adaptor, com_ptr<ID3D12Device1> device)
    : _factory(std::move(factory)), _adaptor(std::move(adaptor)), _device(std::move(device)) {
}

gm::D3d12Device::~D3d12Device() = default;

#endif
