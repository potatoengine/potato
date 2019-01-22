// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/unique_resource.h"
#include "grimm/gpu/device.h"

namespace gm::gpu::d3d11 {
    class DeviceD3D11 final : public Device {
    public:
        DeviceD3D11(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> context);
        virtual ~DeviceD3D11();

        DeviceD3D11(DeviceD3D11&&) = delete;
        DeviceD3D11& operator=(DeviceD3D11&&) = delete;

        static box<Device> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        box<SwapChain> createSwapChain(void* native_window) override;
        box<CommandList> createCommandList(PipelineState* pipelineState = nullptr) override;
        box<PipelineState> createPipelineState(PipelineStateDesc const& desc) override;
        box<Buffer> createBuffer(BufferType type, uint64 size) override;

        void execute(CommandList* commandList) override;

        box<ResourceView> createRenderTargetView(Resource* renderTarget) override;
        box<ResourceView> createShaderResourceView(Buffer* resource) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D11Device> _device;
        com_ptr<ID3D11DeviceContext> _context;
    };
} // namespace gm::gpu::d3d11
