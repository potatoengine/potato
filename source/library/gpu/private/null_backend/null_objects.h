// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/gpu/command_list.h"
#include "potato/gpu/device.h"
#include "potato/gpu/factory.h"
#include "potato/gpu/pipeline_state.h"
#include "potato/gpu/swap_chain.h"
#include "potato/gpu/resource_view.h"
#include "potato/gpu/buffer.h"
#include "potato/gpu/texture.h"
#include "potato/gpu/sampler.h"

namespace up::gpu::null {
    class DeviceNull;

    class FactoryNull final : public GpuDeviceFactory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        rc<GpuDevice> createDevice(int index) override;
    };

    class DeviceNull final : public GpuDevice {
    public:
        rc<GpuSwapChain> createSwapChain(void* native_window) override;
        box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) override;
        box<GpuPipelineState> createPipelineState(PipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(BufferType type, uint64 size) override;
        box<GpuTexture> createTexture2D(TextureDesc const& desc, span<byte const> data) override;
        box<GpuSampler> createSampler() override;

        box<GpuResourceView> createRenderTargetView(GpuTexture* renderTarget) override;
        box<GpuResourceView> createDepthStencilView(GpuTexture* depthStencilBuffer) override;
        box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) override;
        box<GpuResourceView> createShaderResourceView(GpuTexture* texture) override;

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
        box<GpuTexture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class PipelineStateNull final : public GpuPipelineState {
    };

    class CommandListNull final : public GpuCommandList {
    public:
        void setPipelineState(GpuPipelineState* state) override {}

        void clearRenderTarget(GpuResourceView* view, glm::vec4 color) override {}
        void clearDepthStencil(GpuResourceView* view) override {}

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override {}
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override {}

        void finish() override {}
        void clear(GpuPipelineState* pipelineState = nullptr) override {}

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override { return {}; }
        void unmap(GpuBuffer* resource, span<byte const> data) override {}
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override {}

        void bindRenderTarget(uint32 index, GpuResourceView* view) override {}
        void bindDepthStencil(GpuResourceView* view) override {}
        void bindIndexBuffer(GpuBuffer* buffer, IndexType indexType, uint32 offset = 0) override {}
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override {}
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, ShaderStage stage) override {}
        void bindShaderResource(uint32 slot, GpuResourceView* view, ShaderStage stage) override {}
        void bindSampler(uint32 slot, GpuSampler* sampler, ShaderStage stage) override {}
        void setPrimitiveTopology(PrimitiveTopology topology) override {}
        void setViewport(Viewport const& viewport) override {}
        void setClipRect(Rect rect) override {}
    };

    class BufferNull final : public GpuBuffer {
    public:
        BufferNull(BufferType type) : _type(type) {}

        BufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return 0; }

    private:
        BufferType _type;
    };

    class TextureNull final : public GpuTexture {
    public:
        TextureType type() const noexcept override { return TextureType::Texture2D; }
        Format format() const noexcept override { return Format::Unknown; }
        glm::ivec3 dimensions() const noexcept override { return {0, 0, 0}; }
    };

    class SamplerNull final : public GpuSampler {
    };
} // namespace up::gpu::null
