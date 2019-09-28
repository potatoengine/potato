// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/span.h"
#include "potato/spud/rc.h"
#include "potato/spud/vector.h"
#include "potato/spud/int_types.h"
#include "potato/render/gpu_common.h"

namespace up {
    class GpuBuffer;
    class CommandList;
    class GpuDevice;
} // namespace up

namespace up {
    class RenderContext;

    struct MeshBuffer {
        uint32 size = 0;
        uint32 offset = 0;
        uint16 stride = 0;
    };

    struct MeshChannel {
        uint8 buffer = 0;
        GpuFormat format = GpuFormat::R32G32B32Float;
        GpuShaderSemantic semantic = GpuShaderSemantic::Position;
    };

    class Mesh : public shared<Mesh> {
    public:
        UP_RENDER_API explicit Mesh(vector<uint16> indices, vector<up::byte> data, view<MeshBuffer> buffers, view<MeshChannel> channels);
        UP_RENDER_API ~Mesh();

        UP_RENDER_API void populateLayout(span<GpuInputLayoutElement>& inputLayout) const noexcept;
        UP_RENDER_API void updateVertexBuffers(RenderContext& ctx);
        UP_RENDER_API void bindVertexBuffers(RenderContext& ctx);

        uint32 indexCount() const noexcept { return static_cast<uint32>(_indices.size()); }

    private:
        box<GpuBuffer> _ibo;
        box<GpuBuffer> _vbo;
        vector<MeshBuffer> _buffers;
        vector<MeshChannel> _channels;
        vector<uint16> _indices;
        vector<up::byte> _data;
    };
} // namespace up