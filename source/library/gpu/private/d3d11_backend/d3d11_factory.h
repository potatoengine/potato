// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/gpu/factory.h"

namespace up::gpu::d3d11 {
    class FactoryD3D11 final : public Factory {
    public:
        FactoryD3D11(com_ptr<IDXGIFactory2> dxgiFactory);
        virtual ~FactoryD3D11();

        FactoryD3D11(FactoryD3D11&&) = delete;
        FactoryD3D11& operator=(FactoryD3D11&&) = delete;

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        rc<Device> createDevice(int index) override;

    private:
        com_ptr<IDXGIFactory2> _dxgiFactory;
    };
} // namespace up::gpu::d3d11
