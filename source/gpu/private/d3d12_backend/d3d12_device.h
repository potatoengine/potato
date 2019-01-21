// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/device.h"

namespace gm {
    class D3d12Device final : public GpuDevice {
    public:
        D3d12Device(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D12Device1> device, com_ptr<ID3D12CommandQueue> graphicsQueue);
        virtual ~D3d12Device();

        D3d12Device(D3d12Device&&) = delete;
        D3d12Device& operator=(D3d12Device&&) = delete;

        static box<GpuDevice> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        box<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuDescriptorHeap> createDescriptorHeap() override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;

        void execute(GpuCommandList* commandList) override;

        void createRenderTargetView(GpuResource* renderTarget, std::size_t cpuHandle) override;
        box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D12Device1> _device;
        com_ptr<ID3D12CommandQueue> _graphicsQueue;
        com_ptr<ID3D12Fence> _executeFence;
        unique_resource<HANDLE, CloseHandle> _fenceEvent;
    };
} // namespace gm
