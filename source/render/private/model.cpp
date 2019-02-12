// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "model.h"
#include "material.h"
#include "mesh.h"
#include "context.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"
#include <glm/gtc/type_ptr.hpp>

namespace {
    struct alignas(16) Vert {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 uv;
    };

    struct alignas(16) Trans {
        glm::mat4x4 modelWorld;
        glm::mat4x4 worldModel;
    };
} // namespace

gm::Model::Model(rc<Mesh> mesh, rc<Material> material) : _mesh(std::move(mesh)), _material(std::move(material)) {}

gm::Model::~Model() = default;

void GM_VECTORCALL gm::Model::render(RenderContext& ctx, glm::mat4x4 transform) {
    if (_transformBuffer == nullptr) {
        _transformBuffer = ctx.device.createBuffer(gpu::BufferType::Constant, sizeof(Trans));
    }

    Trans trans;
    trans.modelWorld = transpose(transform);
    trans.worldModel = glm::inverse(transform);

    _mesh->updateVertexBuffers(ctx);
    ctx.commandList.update(_transformBuffer.get(), span{&trans, 1}.as_bytes());

    _material->bindMaterialToRender(ctx);
    _mesh->bindVertexBuffers(ctx);
    ctx.commandList.bindConstantBuffer(2, _transformBuffer.get(), gpu::ShaderStage::All);
    ctx.commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
    ctx.commandList.draw(static_cast<uint32>(std::size(cube)));
}
