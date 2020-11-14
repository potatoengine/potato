// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_device.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/unique_resource.h"

namespace up::d3d12 {
    class DeviceD3D12 final : public GpuDevice {
    public:
        DeviceD3D12(
            IDXGIFactoryPtr factory,
            IDXGIAdapterPtr adapter,
            ID3DDevicePtr device);
        virtual ~DeviceD3D12();

        DeviceD3D12(DeviceD3D12&&) = delete;
        DeviceD3D12& operator=(DeviceD3D12&&) = delete;

        static rc<GpuDevice> createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter);

        rc<GpuSwapChain> createSwapChain(void* nativeWindow) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) override;
        rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        void execute(GpuCommandList* commandList) override;

        box<GpuResourceView> createRenderTargetView(GpuTexture* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuTexture* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) override;
        box<GpuResourceView> createShaderResourceView(GpuTexture* texture) override;

    private:
        com_ptr<IDXGIFactory2> _factory;
        com_ptr<IDXGIAdapter1> _adaptor;
        com_ptr<ID3D12Device> _device;
    };
} // namespace up::d3d12
