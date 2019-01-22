// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class PipelineState;
    class Resource;
    class ResourceView;
    class GpuSwapChain;

    struct PipelineStateDesc;

    class Device {
    public:
        Device() = default;
        virtual ~Device() = default;

        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        virtual box<GpuSwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<CommandList> createCommandList(PipelineState* pipelineState = nullptr) = 0;
        virtual box<PipelineState> createPipelineState(PipelineStateDesc const& desc) = 0;
        virtual box<Buffer> createBuffer(BufferType type, uint64 size) = 0;

        virtual void execute(CommandList* commandList) = 0;

        virtual box<ResourceView> createRenderTargetView(Resource* renderTarget) = 0;
        virtual box<ResourceView> createShaderResourceView(Buffer* resource) = 0;
    };
} // namespace gm::gpu
