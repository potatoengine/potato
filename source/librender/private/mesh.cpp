// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/mesh.h"
#include "potato/render/context.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"

up::Mesh::Mesh(vector<up::uint16> indices, vector<up::byte> data, view<MeshBuffer> buffers, view<MeshChannel> channels)
    : _buffers(buffers.begin(), buffers.end()),
      _channels(channels.begin(), channels.end()),
      _indices(std::move(indices)),
      _data(std::move(data)) {
}

up::Mesh::~Mesh() = default;

void up::Mesh::populateLayout(span<GpuInputLayoutElement>& inputLayout) const noexcept {
    auto size = inputLayout.size() < _channels.size() ? inputLayout.size() : _channels.size();

    for (decltype(size) index = 0; index != size; ++index) {
        inputLayout[index].format = _channels[index].format;
        inputLayout[index].semantic = _channels[index].semantic;
        inputLayout[index].semanticIndex = 0; // FIXME
        inputLayout[index].slot = _channels[index].buffer;
    }

    inputLayout = inputLayout.first(size);
}

void up::Mesh::updateVertexBuffers(RenderContext& ctx) {
    if (_ibo == nullptr) {
        _ibo = ctx.device.createBuffer(GpuBufferType::Index, _indices.size() * sizeof(uint16));
        ctx.commandList.update(_ibo.get(), _indices.as_bytes(), 0);
    }
    if (_vbo == nullptr) {
        _vbo = ctx.device.createBuffer(GpuBufferType::Vertex, _data.size());
        ctx.commandList.update(_vbo.get(), _data, 0);
    }
}

void up::Mesh::bindVertexBuffers(RenderContext& ctx) {
    ctx.commandList.bindIndexBuffer(_ibo.get(), GpuIndexFormat::Unsigned16, 0);
    for (int i = 0; i != _buffers.size(); ++i) {
        ctx.commandList.bindVertexBuffer(i, _vbo.get(), _buffers[i].stride, _buffers[i].offset);
    }
}
