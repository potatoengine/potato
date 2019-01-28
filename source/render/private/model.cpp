// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "model.h"
#include "material.h"
#include "mesh.h"
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

gm::Model::Model(rc<Material> material) : _material(std::move(material)) {
    MeshBuffer buffer;
    buffer.stride = sizeof(PackedVector3f) * 2;

    MeshChannel channels[2] = {
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Position},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Color},
    };

    _mesh = make_shared<Mesh>(blob(span{triangle, 6}.as_bytes()), span{&buffer, 1}, channels);
}

gm::Model::~Model() = default;

void gm::Model::render(gpu::CommandList& commandList, gpu::Device& device) {
    _mesh->updateVertexBuffers(commandList, device);

    _material->bindMaterialToRender(commandList, device);
    _mesh->bindVertexBuffers(commandList, device);
    commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
    commandList.draw(3);
}
