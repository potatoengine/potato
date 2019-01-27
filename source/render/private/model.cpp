// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "model.h"
#include "material.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"

static constexpr gm::PackedVector3f triangle[] = {
    {-0.8f, -0.8f, 0},
    {1, 0, 0},
    {0.8f, -0.8f, 0},
    {0, 1, 0},
    {0, +0.8f, 0},
    {0, 0, 1},
};

gm::Model::Model(rc<Material> material) : _material(std::move(material)) {}

gm::Model::~Model() = default;

void gm::Model::render(gpu::CommandList& commandList, gpu::Device& device) {
    if (_vbo == nullptr) {
        _vbo = device.createBuffer(gpu::BufferType::Vertex, sizeof(triangle));
        commandList.update(_vbo.get(), span{triangle, 6}.as_bytes(), 0);
    }

    _material->bindMaterialToRender(commandList, device);
    commandList.bindVertexBuffer(0, _vbo.get(), sizeof(gm::PackedVector3f) * 2);
    commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
    commandList.draw(3);
}
