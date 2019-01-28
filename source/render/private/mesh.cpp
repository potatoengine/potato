// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "mesh.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"

gm::Mesh::Mesh(blob data, view<MeshBuffer> buffers, view<MeshChannel> channels)
    : _data(std::move(data)),
      _buffers(buffers.begin(), buffers.end()),
      _channels(channels.begin(), channels.end()) {
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

void gm::Mesh::updateVertexBuffers(gpu::CommandList& commandList, gpu::Device& device) {
    if (_vbo == nullptr) {
        _vbo = device.createBuffer(gpu::BufferType::Vertex, _data.size());
        commandList.update(_vbo.get(), _data, 0);
    }
}

void gm::Mesh::bindVertexBuffers(gpu::CommandList& commandList, gpu::Device& device) {
    for (int i = 0; i != _buffers.size(); ++i) {
        commandList.bindVertexBuffer(i, _vbo.get(), _buffers[i].stride, _buffers[i].offset);
    }
}
