// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "model.h"
#include "material.h"
#include "mesh.h"
#include "context.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"

namespace {
    struct Vert {
        gm::Packed3 pos;
        gm::Packed3 color;
    };
} // namespace

static const float z = -5;
static const Vert triangle[] = {
    {{-0.5f, -0.5f, z},
     {1, 0, 0}},
    {{+0.5f, -0.5f, z},
     {0, 1, 0}},
    {{+0.5f, +0.5f, z},
     {0, 0, 1}},
    {{+0.5f, +0.5f, z},
     {0, 0, 1}},
    {{-0.5f, +0.5f, z},
     {0, 1, 0}},
    {{-0.5f, -0.5f, z},
     {1, 0, 0}},
};

gm::Model::Model(rc<Material> material) : _material(std::move(material)) {
    MeshBuffer buffer;
    buffer.stride = sizeof(Vert);

    MeshChannel channels[2] = {
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Position},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Color},
    };

    _mesh = make_shared<Mesh>(blob(span{triangle, std::size(triangle)}.as_bytes()), span{&buffer, 1}, channels);
}

gm::Model::~Model() = default;

void gm::Model::render(RenderContext& ctx) {
    _mesh->updateVertexBuffers(ctx);

    _material->bindMaterialToRender(ctx);
    _mesh->bindVertexBuffers(ctx);
    ctx.commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
    ctx.commandList.draw(static_cast<uint32>(std::size(triangle)));
}
