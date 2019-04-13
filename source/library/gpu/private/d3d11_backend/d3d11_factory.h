// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/gpu/factory.h"

namespace up::d3d11 {
    class FactoryD3D11 final : public GpuDeviceFactory {
    public:
        FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory);
        virtual ~FactoryD3D11();

        FactoryD3D11(FactoryD3D11&&) = delete;
        FactoryD3D11& operator=(FactoryD3D11&&) = delete;

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(GpuDeviceInfo const&)> callback) override;
        rc<GpuDevice> createDevice(int index) override;

    private:
        com_ptr<IDXGIFactory2> _dxgiFactory;
    };
} // namespace up::d3d11
