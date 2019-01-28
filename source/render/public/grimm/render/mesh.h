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
        GM_RENDER_API explicit Mesh(blob data, view<MeshBuffer> buffers, view<MeshChannel> channels);
        GM_RENDER_API ~Mesh();

        GM_RENDER_API void populateLayout(span<gpu::InputLayoutElement>& inputLayout) const noexcept;
        GM_RENDER_API void updateVertexBuffers(gpu::CommandList& commandList, gpu::Device& device);
        GM_RENDER_API void bindVertexBuffers(gpu::CommandList& commandList, gpu::Device& device);

    private:
        box<gpu::Buffer> _vbo;
        vector<MeshBuffer> _buffers;
        vector<MeshChannel> _channels;
        blob _data;
    };
} // namespace gm
