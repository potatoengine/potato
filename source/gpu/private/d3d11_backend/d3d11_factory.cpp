// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_factory.h"
#include "d3d11_device.h"
#include "d3d11_platform.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"

gm::gpu::d3d11::FactoryD3D11::FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory)
    : _dxgiFactory(std::move(dxgiFactory)) {
    GM_ASSERT(_dxgiFactory != nullptr);
}

gm::gpu::d3d11::FactoryD3D11::~FactoryD3D11() = default;

#if GM_GPU_ENABLE_D3D11
auto gm::gpu::CreateGPUFactoryD3D11() -> box<gpu::GpuDeviceFactory> {
    com_ptr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory2), out_ptr(dxgiFactory));
    return make_box<d3d11::FactoryD3D11>(std::move(dxgiFactory));
}
#endif

bool gm::gpu::d3d11::FactoryD3D11::isEnabled() const {
    return true;
}

void gm::gpu::d3d11::FactoryD3D11::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    com_ptr<IDXGIAdapter1> adapter;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        DeviceInfo info = {index};
        callback(info);
    }
}

auto gm::gpu::d3d11::FactoryD3D11::createDevice(int index) -> box<gpu::GpuDevice> {
    com_ptr<IDXGIAdapter1> adapter;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        if (targetIndex == index) {
            return DeviceD3D11::createDevice(_dxgiFactory, std::move(adapter));
        }
    }

    return nullptr;
}
