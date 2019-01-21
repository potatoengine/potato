// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/types.h"
#include "grimm/math/packed.h"
#include "transition.h"

namespace gm {
    class GpuResource;
    class GpuPipelineState;
    class GpuResourceView;
    class GpuBuffer;

    class GpuCommandList {
    public:
        GpuCommandList() = default;
        virtual ~GpuCommandList() = default;

        GpuCommandList(GpuCommandList&&) = delete;
        GpuCommandList& operator=(GpuCommandList&&) = delete;

        virtual void setPipelineState(GpuPipelineState* state) = 0;

        virtual void bindRenderTarget(uint32 index, GpuResourceView* view) = 0;
        virtual void bindBuffer(uint32 slot, GpuBuffer* buffer, uint64 stride, uint64 offset = 0) = 0;
        virtual void bindShaderResource(uint32 slot, GpuResourceView* view) = 0;
        virtual void setPrimitiveTopology(PrimitiveTopology topology) = 0;

        virtual void draw(uint32 vertexCount, uint32 firstVertex = 0) = 0;

        virtual void clearRenderTarget(GpuResourceView* view, PackedVector4f color) = 0;

        virtual void finish() = 0;
        virtual void clear(GpuPipelineState* pipelineState = nullptr) = 0;

        virtual span<byte> map(GpuBuffer* resource, uint64 size, uint64 offset = 0) = 0;
        virtual void unmap(GpuBuffer* resource, span<byte const> data) = 0;
        virtual void update(GpuBuffer* resource, span<byte const> data, uint64 offset = 0) = 0;
    };
} // namespace gm
