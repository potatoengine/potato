// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/types.h"
#include "grimm/math/packed.h"

namespace gm::gpu {
    class Buffer;
    class ResourceView;
    class PipelineState;
    class Sampler;
    class Texture;

    class CommandList {
    public:
        CommandList() = default;
        virtual ~CommandList() = default;

        CommandList(CommandList&&) = delete;
        CommandList& operator=(CommandList&&) = delete;

        virtual void setPipelineState(PipelineState* state) = 0;

        virtual void bindRenderTarget(uint32 index, ResourceView* view) = 0;
        virtual void bindIndexBuffer(Buffer* buffer, IndexType indexType, uint32 offset = 0) = 0;
        virtual void bindVertexBuffer(uint32 slot, Buffer* buffer, uint64 stride, uint64 offset = 0) = 0;
        virtual void bindConstantBuffer(uint32 slot, Buffer* buffer, ShaderStage stage) = 0;
        virtual void bindShaderResource(uint32 slot, ResourceView* view, ShaderStage stage) = 0;
        virtual void bindSampler(uint32 slot, Sampler* sampler, ShaderStage stage) = 0;
        virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;
        virtual void setViewport(Viewport const& viewport) = 0;
        virtual void setClipRect(Rect rect) = 0;

        virtual void draw(uint32 vertexCount, uint32 firstVertex = 0) = 0;
        virtual void drawIndexed(uint32 indexCount, uint32 firstIndex = 0, uint32 baseIndex = 0) = 0;

        virtual void clearRenderTarget(ResourceView* view, Packed4 color) = 0;

        virtual void finish() = 0;
        virtual void clear(PipelineState* pipelineState = nullptr) = 0;

        virtual span<byte> map(Buffer* resource, uint64 size, uint64 offset = 0) = 0;
        virtual void unmap(Buffer* resource, span<byte const> data) = 0;
        virtual void update(Buffer* resource, span<byte const> data, uint64 offset = 0) = 0;
    };
} // namespace gm::gpu
