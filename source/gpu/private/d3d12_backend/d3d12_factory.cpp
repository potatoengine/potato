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
    com_ptr<IDXGIAdapter1> adaptor;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adaptor)) == S_OK) {
        DeviceInfo info = {index};
        callback(info);
    }
}

auto gm::D3d12Factory::createDevice(int index) -> box<IGPUDevice> {
    com_ptr<IDXGIAdapter1> adaptor;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adaptor)) == S_OK) {
        if (targetIndex == index) {
            com_ptr<ID3D12Device1> device;
            D3D12CreateDevice(adaptor.get(), D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device1), out_ptr(device));
            if (device.empty()) {
                return nullptr;
            }

            D3D12_COMMAND_QUEUE_DESC desc = {0};
            desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

            com_ptr<ID3D12CommandQueue> graphicsQueue;
            device->CreateCommandQueue(&desc, __uuidof(ID3D12CommandQueue), out_ptr(graphicsQueue));

            return make_box<D3d12Device>(_dxgiFactory, std::move(adaptor), std::move(device), std::move(graphicsQueue));
        }
    }

    return nullptr;
}

#endif
