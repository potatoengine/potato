// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "potato/foundation/box.h"
#include "potato/foundation/int_types.h"
#include "potato/foundation/rc.h"

namespace up::gpu {
    class GpuBuffer;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class GpuSwapChain;
    class Texture;

    struct PipelineStateDesc;
    struct TextureDesc;

    class GpuDevice : public shared<GpuDevice> {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual rc<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(PipelineStateDesc const& desc) = 0;
        virtual box<GpuBuffer> createBuffer(BufferType type, uint64 size) = 0;
        virtual box<Texture> createTexture2D(TextureDesc const& desc, span<byte const> data) = 0;
        virtual box<GpuSampler> createSampler() = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(Texture* renderTarget) = 0;
        virtual box<GpuResourceView> createDepthStencilView(Texture* depthStencilBuffer) = 0;
        virtual box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) = 0;
        virtual box<GpuResourceView> createShaderResourceView(Texture* texture) = 0;
    };
} // namespace up::gpu
