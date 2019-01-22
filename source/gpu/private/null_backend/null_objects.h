// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "command_list.h"
#include "device.h"
#include "factory.h"
#include "pipeline_state.h"
#include "resource.h"
#include "swap_chain.h"
#include "resource.h"
#include "resource_view.h"
#include "buffer.h"

namespace gm::gpu::null {
    class DeviceNull;

    class FactoryNull final : public GpuDeviceFactory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        box<GpuDevice> createDevice(int index) override;
    };

    class DeviceNull final : public GpuDevice {
    public:
        box<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) override;
        box<Buffer> createBuffer(BufferType type, uint64 size) override;

        box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) override;
        box<GpuResourceView> createShaderResourceView(Buffer* resource) override;

        void execute(GpuCommandList* commands) override {}
    };

    class ResourceViewNull final : public GpuResourceView {
    public:
        ResourceViewNull(ViewType type) : _type(type) {}

        ViewType type() const override { return _type; }

    private:
        ViewType _type;
    };

    class SwapChainNull final : public GpuSwapChain {
    public:
        void present() override {}
        void resizeBuffers(int width, int height) override {}
        box<GpuResource> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class PipelineStateNull final : public GpuPipelineState {
    };

    class ResourceNull final : public GpuResource {
    };

    class CommandListNull final : public GpuCommandList {
    public:
        void setPipelineState(GpuPipelineState* state) override {}

        void clearRenderTarget(GpuResourceView* view, PackedVector4f color) override {}

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override {}

        void finish() override {}
        void clear(GpuPipelineState* pipelineState = nullptr) override {}

        span<byte> map(Buffer* resource, uint64 size, uint64 offset = 0) override { return {}; }
        void unmap(Buffer* resource, span<byte const> data) override {}
        void update(Buffer* resource, span<byte const> data, uint64 offset = 0) override {}

        void bindRenderTarget(uint32 index, GpuResourceView* view) override {}
        void bindBuffer(uint32 slot, Buffer* buffer, uint64 stride, uint64 offset = 0) override {}
        void bindShaderResource(uint32 slot, GpuResourceView* view) override {}
        void setPrimitiveTopology(PrimitiveTopology topology) override {}
        void setViewport(Viewport const& viewport) override {}
    };

    class BufferNull final : public Buffer {
    public:
        BufferNull(BufferType type) : _type(type) {}

        BufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return 0; }

    private:
        BufferType _type;
    };
} // namespace gm::gpu::null
