// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/gpu/device.h"
#include "com_ptr.h"
#include "direct3d.h"

namespace gm
{
    class D3d12Device final : public IGPUDevice {
    public:
        D3d12Device(com_ptr<IDXGIFactory1>, com_ptr<IDXGIAdapter1> adaptor, com_ptr<ID3D12Device1> device);
        virtual ~D3d12Device();

        D3d12Device(D3d12Device&&) = delete;
        D3d12Device& operator=(D3d12Device&) = delete;

    private:
        com_ptr<IDXGIFactory1> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D12Device1> _device;
    };
}
