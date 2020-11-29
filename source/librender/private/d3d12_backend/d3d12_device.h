// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_device.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/unique_resource.h"

namespace up {
    class GpuRenderable;
};

namespace up::d3d12 {
    class CommandListD3D12;

    class DeviceD3D12 final : public GpuDevice {
    public:
        DeviceD3D12(
            IDXGIFactoryPtr factory,
            IDXGIAdapterPtr adapter);
        virtual ~DeviceD3D12();

        DeviceD3D12(DeviceD3D12&&) = delete;
        DeviceD3D12& operator=(DeviceD3D12&&) = delete;

        static rc<GpuDevice> createDevice(IDXGIFactoryPtr factory, IDXGIAdapterPtr adapter);

        box<GpuRenderable> createRenderable(IRenderable* pInterface) override;
        rc<GpuSwapChain> createSwapChain(void* nativeWindow) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) override;
        rc<GpuTexture> createRenderTarget(GpuTextureDesc const& desc, GpuSwapChain* swapChain) override;
        rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        bool create();
        void render(const FrameData& frameData, GpuRenderable* renderable) override;
        void execute();

    protected:
        void createAllocator(); 
        void createFrameSync();
        void waitForFrame();

    private:
        IDXGIFactoryPtr _factory;
        IDXGIAdapterPtr _adapter;
        ID3DDevicePtr _device;

        ID3DDescriptorHeapPtr _rtvHeap;
        ID3DDescriptorHeapPtr _srvUavHeap;

        ID3DCommandQueuePtr _commandQueue;
      
        uint32 _rtvDescriptorSize = 0;
        uint32 _srvUavDescriptorSize = 0;

        // main command list -- for now we use single command list but that will most likely change
        // in the future
        box<CommandListD3D12> _commandList; 

        // sync objects
        HANDLE _swapChainEvent;
        ID3DFencePtr _renderFrameFence;
        uint64 _renderContextFenceValue;
        HANDLE _renderContextFenceEvent;

        // allocator
        D3D12MA::Allocator* _allocator;

    };
} // namespace up::d3d12
