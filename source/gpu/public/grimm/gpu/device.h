// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"

namespace gm {
    class GpuSwapChain;
    class GpuDescriptorHeap;
    class GpuResource;
    class GpuPipelineState;
    class GpuCommandList;
    class GpuResourceView;
    class GpuBuffer;

    struct GpuPipelineStateDesc;
} // namespace gm

namespace gm::gpu {
    class GpuDevice {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual box<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;
        virtual box<GpuBuffer> createBuffer(BufferType type, uint64 size) = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) = 0;
        virtual box<GpuResourceView> createShaderResourceView(GpuBuffer* resource) = 0;
    };
} // namespace gm::gpu
