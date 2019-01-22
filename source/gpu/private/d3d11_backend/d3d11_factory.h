// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/gpu/factory.h"

namespace gm {
    class FactoryD3D11 final : public gpu::GpuDeviceFactory {
    public:
        FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory);
        virtual ~FactoryD3D11();

        FactoryD3D11(FactoryD3D11&&) = delete;
        FactoryD3D11& operator=(FactoryD3D11&&) = delete;

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<gpu::GpuDevice> createDevice(int index) override;

    private:
        com_ptr<IDXGIFactory2> _dxgiFactory;
    };
} // namespace gm
