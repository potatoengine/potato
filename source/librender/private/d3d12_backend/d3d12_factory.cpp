// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_factory.h"
#include "d3d12_device.h"
#include "d3d12_platform.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

up::d3d12::FactoryD3D12::FactoryD3D12(IDXGIFactoryPtr dxgiFactory) : _dxgiFactory(std::move(dxgiFactory)) {
    UP_ASSERT(_dxgiFactory != nullptr);
}

up::d3d12::FactoryD3D12::~FactoryD3D12() = default;

#if defined(UP_GPU_ENABLE_D3D12)
auto up::CreateFactoryD3D12() -> box<GpuDeviceFactory> {

    UINT dxgiFactoryFlags = 0;

#    if defined(_DEBUG)
    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
    {
        com_ptr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
            debugController->EnableDebugLayer();

            // Enable additional debug layers.
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }
#    endif

    DXGIFactoryPtr dxgiFactory;
    CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory));
    return new_box<d3d12::FactoryD3D12>(std::move(dxgiFactory));
}
#endif

bool up::d3d12::FactoryD3D12::isEnabled() const {
    return true;
}

void up::d3d12::FactoryD3D12::enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) {
    com_ptr<DXGIAdapterType> adapter;

    int index = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        GpuDeviceInfo info = {index};
        callback(info);
    }
}

auto up::d3d12::FactoryD3D12::createDevice(int index) -> rc<GpuDevice> {
    com_ptr<DXGIAdapterType> adapter;

    UINT targetIndex = 0;
    while (_dxgiFactory->EnumAdapters1(index, out_ptr(adapter)) == S_OK) {
        if (targetIndex == index) {
            return DeviceD3D12::createDevice(_dxgiFactory, std::move(adapter));
        }
    }

    return nullptr;
}
