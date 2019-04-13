// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_factory.h"
#include "d3d11_device.h"
#include "d3d11_platform.h"
#include "potato/foundation/assertion.h"
#include "potato/foundation/out_ptr.h"

up::gpu::d3d11::FactoryD3D11::FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory)
    : _dxgiFactory(std::move(dxgiFactory)) {
    UP_ASSERT(_dxgiFactory != nullptr);
}

up::gpu::d3d11::FactoryD3D11::~FactoryD3D11() = default;

#if UP_GPU_ENABLE_D3D11
auto up::gpu::CreateFactoryD3D11() -> box<gpu::GpuDeviceFactory> {
    com_ptr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory2), out_ptr(dxgiFactory));
    return new_box<d3d11::FactoryD3D11>(std::move(dxgiFactory));
}
#endif

bool up::gpu::d3d11::FactoryD3D11::isEnabled() const {
    return true;
}

void up::gpu::d3d11::FactoryD3D11::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    com_ptr<IDXGIAdapter1> adapter;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        DeviceInfo info = {index};
        callback(info);
    }
}

auto up::gpu::d3d11::FactoryD3D11::createDevice(int index) -> rc<gpu::GpuDevice> {
    com_ptr<IDXGIAdapter1> adapter;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        if (targetIndex == index) {
            return DeviceD3D11::createDevice(_dxgiFactory, std::move(adapter));
        }
    }

    return nullptr;
}
