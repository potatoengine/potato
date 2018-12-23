// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/gpu/device.h"

namespace gm {
    class D3d12Device final : public IGPUDevice {
    public:
        D3d12Device(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D12Device1> device, com_ptr<ID3D12CommandQueue> graphicsQueue);
        virtual ~D3d12Device();

        D3d12Device(D3d12Device&&) = delete;
        D3d12Device& operator=(D3d12Device&&) = delete;

        static box<IGPUDevice> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        box<ISwapChain> createSwapChain(void* native_window) override;
        box<IDescriptorHeap> createDescriptorHeap() override;
        void createRenderTargetView(IGpuResource* renderTarget, std::size_t cpuHandle) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D12Device1> _device;
        com_ptr<ID3D12CommandQueue> _graphicsQueue;
    };
} // namespace gm
