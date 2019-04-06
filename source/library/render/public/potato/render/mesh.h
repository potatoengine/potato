// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/span.h"
#include "potato/foundation/rc.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/int_types.h"
#include "potato/gpu/common.h"

namespace up::gpu {
    class Buffer;
    class CommandList;
    class Device;
} // namespace up::gpu

namespace up {
    class RenderContext;

    struct MeshBuffer {
        uint32 size = 0;
        uint32 offset = 0;
        uint16 stride = 0;
    };

    struct MeshChannel {
        uint8 buffer = 0;
        gpu::Format format = gpu::Format::R32G32B32Float;
        gpu::Semantic semantic = gpu::Semantic::Position;
    };

    class Mesh : public shared<Mesh> {
    public:
        UP_RENDER_API explicit Mesh(vector<uint16> indices, vector<up::byte> data, view<MeshBuffer> buffers, view<MeshChannel> channels);
        UP_RENDER_API ~Mesh();

        UP_RENDER_API void populateLayout(span<gpu::InputLayoutElement>& inputLayout) const noexcept;
        UP_RENDER_API void updateVertexBuffers(RenderContext& ctx);
        UP_RENDER_API void bindVertexBuffers(RenderContext& ctx);

        uint32 indexCount() const noexcept { return static_cast<uint32>(_indices.size()); }

    private:
        box<gpu::Buffer> _ibo;
        box<gpu::Buffer> _vbo;
        vector<MeshBuffer> _buffers;
        vector<MeshChannel> _channels;
        vector<uint16> _indices;
        vector<up::byte> _data;
    };
} // namespace up
