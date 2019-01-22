// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/device.h"

namespace gm {
    class DeviceD3D11 final : public gpu::GpuDevice {
    public:
        DeviceD3D11(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> context);
        virtual ~DeviceD3D11();

        DeviceD3D11(DeviceD3D11&&) = delete;
        DeviceD3D11& operator=(DeviceD3D11&&) = delete;

        static box<GpuDevice> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        box<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(BufferType type, uint64 size) override;

        void execute(GpuCommandList* commandList) override;

        box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) override;
        box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D11Device> _device;
        com_ptr<ID3D11DeviceContext> _context;
    };
} // namespace gm
