// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/foundation/box.h"
#include "potato/foundation/int_types.h"
#include "potato/foundation/rc.h"

namespace up::gpu {
    class GpuBuffer;
    class GpuCommandList;
    class PipelineState;
    class ResourceView;
    class Sampler;
    class SwapChain;
    class Texture;

    struct PipelineStateDesc;
    struct TextureDesc;

    class Device : public shared<Device> {
    public:
        Device() = default;
        virtual ~Device() = default;

        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        virtual rc<SwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(PipelineState* pipelineState = nullptr) = 0;
        virtual box<PipelineState> createPipelineState(PipelineStateDesc const& desc) = 0;
        virtual box<GpuBuffer> createBuffer(BufferType type, uint64 size) = 0;
        virtual box<Texture> createTexture2D(TextureDesc const& desc, span<byte const> data) = 0;
        virtual box<Sampler> createSampler() = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<ResourceView> createRenderTargetView(Texture* renderTarget) = 0;
        virtual box<ResourceView> createDepthStencilView(Texture* depthStencilBuffer) = 0;
        virtual box<ResourceView> createShaderResourceView(GpuBuffer* resource) = 0;
        virtual box<ResourceView> createShaderResourceView(Texture* texture) = 0;
    };
} // namespace up::gpu
