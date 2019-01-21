// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"

namespace gm {
    class GpuSwapChain;
    class GpuDescriptorHeap;
    class GpuResource;
    class GpuPipelineState;
    class GpuCommandList;
    class GpuResourceView;

    struct GpuPipelineStateDesc;

    class GpuDevice {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual box<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<GpuCommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;

        virtual void execute(GpuCommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) = 0;
    };
} // namespace gm
