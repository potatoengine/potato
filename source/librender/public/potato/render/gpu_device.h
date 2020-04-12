// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "gpu_common.h"
#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/rc.h"

namespace up {
    class GpuBuffer;
    class GpuCommandList;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuSampler;
    class GpuSwapChain;
    class GpuTexture;

    struct GpuPipelineStateDesc;
    struct GpuTextureDesc;

    class GpuDevice : public shared<GpuDevice> {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual rc<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;
        virtual box<GpuBuffer> createBuffer(GpuBufferType type, uint64 size) = 0;
        virtual rc<GpuTexture> createTexture2D(GpuTextureDesc const& desc, span<byte const> data) = 0;
        virtual box<GpuSampler> createSampler() = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(GpuTexture* renderTarget) = 0;
        virtual box<GpuResourceView> createDepthStencilView(GpuTexture* depthStencilBuffer) = 0;
        virtual box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) = 0;
        virtual box<GpuResourceView> createShaderResourceView(GpuTexture* texture) = 0;
    };
} // namespace up
