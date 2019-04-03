// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/render/mesh.h"
#include "grimm/render/context.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"

gm::Mesh::Mesh(vector<gm::uint16> indices, vector<gm::byte> data, view<MeshBuffer> buffers, view<MeshChannel> channels)
    : _data(std::move(data)),
      _buffers(buffers.begin(), buffers.end()),
      _channels(channels.begin(), channels.end()),
      _indices(std::move(indices)) {
}

gm::Mesh::~Mesh() = default;

void gm::Mesh::populateLayout(span<gpu::InputLayoutElement>& inputLayout) const noexcept {
    auto size = inputLayout.size() < _channels.size() ? inputLayout.size() : _channels.size();

    for (decltype(size) index = 0; index != size; ++index) {
        inputLayout[index].format = _channels[index].format;
        inputLayout[index].semantic = _channels[index].semantic;
        inputLayout[index].semanticIndex = 0; // FIXME
        inputLayout[index].slot = _channels[index].buffer;
    }

    inputLayout = inputLayout.first(size);
}

void gm::Mesh::updateVertexBuffers(RenderContext& ctx) {
    if (_ibo == nullptr) {
        _ibo = ctx.device.createBuffer(gpu::BufferType::Index, _indices.size() * sizeof(uint16));
        ctx.commandList.update(_ibo.get(), span{_indices.data(), _indices.size()}.as_bytes(), 0);
    }
    if (_vbo == nullptr) {
        _vbo = ctx.device.createBuffer(gpu::BufferType::Vertex, _data.size());
        ctx.commandList.update(_vbo.get(), _data, 0);
    }
}

void gm::Mesh::bindVertexBuffers(RenderContext& ctx) {
    ctx.commandList.bindIndexBuffer(_ibo.get(), gpu::IndexType::Unsigned16, 0);
    for (int i = 0; i != _buffers.size(); ++i) {
        ctx.commandList.bindVertexBuffer(i, _vbo.get(), _buffers[i].stride, _buffers[i].offset);
    }
}
