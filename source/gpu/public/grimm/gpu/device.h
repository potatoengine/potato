// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/types.h"
#include "grimm/foundation/rc.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class PipelineState;
    class ResourceView;
    class Sampler;
    class SwapChain;
    class Texture;

    struct PipelineStateDesc;

    class Device : public shared<Device> {
    public:
        Device() = default;
        virtual ~Device() = default;

        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        virtual box<SwapChain> createSwapChain(void* nativeWindow) = 0;
        virtual box<CommandList> createCommandList(PipelineState* pipelineState = nullptr) = 0;
        virtual box<PipelineState> createPipelineState(PipelineStateDesc const& desc) = 0;
        virtual box<Buffer> createBuffer(BufferType type, uint64 size) = 0;
        virtual box<Texture> createTexture2D(uint32 width, uint32 height, Format format, span<byte const> data) = 0;
        virtual box<Sampler> createSampler() = 0;

        virtual void execute(CommandList* commandList) = 0;

        virtual box<ResourceView> createRenderTargetView(Texture* renderTarget) = 0;
        virtual box<ResourceView> createShaderResourceView(Buffer* resource) = 0;
        virtual box<ResourceView> createShaderResourceView(Texture* texture) = 0;
    };
} // namespace gm::gpu
