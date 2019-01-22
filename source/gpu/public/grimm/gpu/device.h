// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class GpuPipelineState;
    class GpuResource;
    class GpuResourceView;
    class GpuSwapChain;

    struct GpuPipelineStateDesc;

    class GpuDevice {
    public:
        GpuDevice() = default;
        virtual ~GpuDevice() = default;

        GpuDevice(GpuDevice&&) = delete;
        GpuDevice& operator=(GpuDevice&&) = delete;

        virtual box<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<CommandList> createCommandList(GpuPipelineState* pipelineState = nullptr) = 0;
        virtual box<GpuPipelineState> createPipelineState(GpuPipelineStateDesc const& desc) = 0;
        virtual box<Buffer> createBuffer(BufferType type, uint64 size) = 0;

        virtual void execute(CommandList* commandList) = 0;

        virtual box<GpuResourceView> createRenderTargetView(GpuResource* renderTarget) = 0;
        virtual box<GpuResourceView> createShaderResourceView(Buffer* resource) = 0;
    };
} // namespace gm::gpu
