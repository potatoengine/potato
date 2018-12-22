// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_factory.h"
#    include "d3d12_device.h"
#    include "direct3d.h"
#    include "grimm/foundation/assert.h"
#    include "grimm/foundation/out_ptr.h"

gm::D3d12Factory::D3d12Factory(com_ptr<IDXGIFactory2> dxgiFactory)
    : _dxgiFactory(std::move(dxgiFactory)) {
    GM_ASSERT(_dxgiFactory != nullptr);
}

gm::D3d12Factory::~D3d12Factory() = default;

auto gm::CreateD3d12GPUFactory() -> box<IGPUFactory> {
    com_ptr<IDXGIFactory2> dxgiFactory;
    CreateDXGIFactory1(__uuidof(IDXGIFactory2), out_ptr(dxgiFactory));
    return make_box<D3d12Factory>(std::move(dxgiFactory));
}

bool gm::D3d12Factory::isEnabled() const {
    return true;
}

void gm::D3d12Factory::enumerateDevices(delegate<void(DeviceInfo const&)> callback) {
    com_ptr<IDXGIAdapter1> adapter;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        DeviceInfo info = {index};
        callback(info);
    }
}

auto gm::D3d12Factory::createDevice(int index) -> box<IGPUDevice> {
    com_ptr<IDXGIAdapter1> adapter;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        if (targetIndex == index) {
            return D3d12Device::createDevice(_dxgiFactory, std::move(adapter));
        }
    }

    return nullptr;
}

#endif
