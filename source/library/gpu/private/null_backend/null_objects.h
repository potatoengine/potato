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

    class FactoryNull final : public Factory {
    public:
        bool isEnabled() const override { return true; }
        void enumerateDevices(delegate<void(DeviceInfo const&)> callback) override;
        rc<Device> createDevice(int index) override;
    };

    class DeviceNull final : public Device {
    public:
        rc<SwapChain> createSwapChain(void* native_window) override;
        box<CommandList> createCommandList(PipelineState* pipelineState = nullptr) override;
        box<PipelineState> createPipelineState(PipelineStateDesc const& desc) override;
        box<GpuBuffer> createBuffer(BufferType type, uint64 size) override;
        box<Texture> createTexture2D(TextureDesc const& desc, span<byte const> data) override;
        box<Sampler> createSampler() override;

        box<ResourceView> createRenderTargetView(Texture* renderTarget) override;
        box<ResourceView> createDepthStencilView(Texture* depthStencilBuffer) override;
        box<ResourceView> createShaderResourceView(GpuBuffer* resource) override;
        box<ResourceView> createShaderResourceView(Texture* texture) override;

        void execute(CommandList* commands) override {}
    };

    class ResourceViewNull final : public ResourceView {
    public:
        ResourceViewNull(ViewType type) : _type(type) {}

        ViewType type() const override { return _type; }

    private:
        ViewType _type;
    };

    class SwapChainNull final : public SwapChain {
    public:
        void present() override {}
        void resizeBuffers(int width, int height) override {}
        box<Texture> getBuffer(int index) override;
        int getCurrentBufferIndex() override;
    };

    class PipelineStateNull final : public PipelineState {
    };

    class CommandListNull final : public CommandList {
    public:
        void setPipelineState(PipelineState* state) override {}

        void clearRenderTarget(ResourceView* view, glm::vec4 color) override {}
        void clearDepthStencil(ResourceView* view) override {}

        void draw(uint32 vertexCount, uint32 firstVertex = 0) override {}
        void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) override {}

        void finish() override {}
        void clear(PipelineState* pipelineState = nullptr) override {}

        span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) override { return {}; }
        void unmap(GpuBuffer* resource, span<byte const> data) override {}
        void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) override {}

        void bindRenderTarget(uint32 index, ResourceView* view) override {}
        void bindDepthStencil(ResourceView* view) override {}
        void bindIndexBuffer(GpuBuffer* buffer, IndexType indexType, uint32 offset = 0) override {}
        void bindVertexBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) override {}
        void bindConstantBuffer(uint32 slot, GpuBuffer* buffer, ShaderStage stage) override {}
        void bindShaderResource(uint32 slot, ResourceView* view, ShaderStage stage) override {}
        void bindSampler(uint32 slot, Sampler* sampler, ShaderStage stage) override {}
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

    class TextureNull final : public Texture {
    public:
        TextureType type() const noexcept override { return TextureType::Texture2D; }
        Format format() const noexcept override { return Format::Unknown; }
        glm::ivec3 dimensions() const noexcept override { return {0, 0, 0}; }
    };

    class SamplerNull final : public Sampler {
    };
} // namespace up::gpu::null
