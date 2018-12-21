// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/gpu/factory.h"

namespace gm {
    class D3d12Factory final : public IGPUFactory {
    public:
        D3d12Factory(com_ptr<IDXGIFactory1> dxgiFactory);
        virtual ~D3d12Factory();

        D3d12Factory(D3d12Factory&&) = delete;
        D3d12Factory& operator=(D3d12Factory&) = delete;

        bool isEnabled() const override;
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<IGPUDevice> createDevice(int index) override;

    private:
        com_ptr<IDXGIFactory1> _dxgiFactory;
    };

    box<IGPUFactory> CreateD3d12Factory();
} // namespace gm
