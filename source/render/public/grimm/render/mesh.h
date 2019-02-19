// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/rc.h"
#include "grimm/foundation/vector.h"
#include "grimm/gpu/common.h"

namespace gm::gpu {
    class Buffer;
    class CommandList;
    class Device;
} // namespace gm::gpu

namespace gm {
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
        GM_RENDER_API explicit Mesh(vector<uint16> indices, vector<byte> data, view<MeshBuffer> buffers, view<MeshChannel> channels);
        GM_RENDER_API ~Mesh();

        GM_RENDER_API void populateLayout(span<gpu::InputLayoutElement>& inputLayout) const noexcept;
        GM_RENDER_API void updateVertexBuffers(RenderContext& ctx);
        GM_RENDER_API void bindVertexBuffers(RenderContext& ctx);

        uint32 indexCount() const noexcept { return static_cast<uint32>(_indices.size()); }

    private:
        box<gpu::Buffer> _ibo;
        box<gpu::Buffer> _vbo;
        vector<MeshBuffer> _buffers;
        vector<MeshChannel> _channels;
        vector<uint16> _indices;
        vector<byte> _data;
    };
} // namespace gm
