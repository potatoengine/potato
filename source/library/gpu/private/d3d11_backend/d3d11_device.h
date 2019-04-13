// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/unique_resource.h"
#include "potato/gpu/device.h"

namespace up::gpu::d3d11 {
    class DeviceD3D11 final : public Device {
    public:
        DeviceD3D11(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter, com_ptr<ID3D11Device> device, com_ptr<ID3D11DeviceContext> context);
        virtual ~DeviceD3D11();

        DeviceD3D11(DeviceD3D11&&) = delete;
        DeviceD3D11& operator=(DeviceD3D11&&) = delete;

        static rc<Device> createDevice(com_ptr<IDXGIFactory2> factory, com_ptr<IDXGIAdapter1> adapter);

        rc<SwapChain> createSwapChain(void* native_window) override;
        box<CommandList> createCommandList(PipelineState* pipelineState = nullptr) override;
        box<PipelineState> createPipelineState(PipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(BufferType type, uint64 size) override;
        box<Texture> createTexture2D(TextureDesc const& desc, span<byte const> data) override;
        box<Sampler> createSampler() override;

        void execute(CommandList* commandList) override;

        box<ResourceView> createRenderTargetView(Texture* renderTarget) override;
        box<ResourceView> createDepthStencilView(Texture* depthStencilBuffer) override;
        box<ResourceView> createShaderResourceView(GpuBuffer* resource) override;
        box<ResourceView> createShaderResourceView(Texture* texture) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D11Device> _device;
        com_ptr<ID3D11DeviceContext> _context;
    };
} // namespace up::gpu::d3d11
