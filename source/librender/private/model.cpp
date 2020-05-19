// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/render/model.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/context.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_command_list.h"
#include <glm/gtc/type_ptr.hpp>

namespace {
    struct alignas(16) Vert {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 uv;
    };

    struct alignas(16) Trans {
        glm::mat4x4 modelWorld;
        glm::mat4x4 worldModel;
    };
} // namespace

up::Model::Model(rc<Mesh> mesh, rc<Material> material) : _material(std::move(material)), _mesh(std::move(mesh)) {}

up::Model::~Model() = default;

void UP_VECTORCALL up::Model::render(RenderContext& ctx, glm::mat4x4 transform) {
    if (_transformBuffer == nullptr) {
        _transformBuffer = ctx.device.createBuffer(GpuBufferType::Constant, sizeof(Trans));
    }

    Trans trans;
    trans.modelWorld = transpose(transform);
    trans.worldModel = glm::inverse(transform);

    _mesh->updateVertexBuffers(ctx);
    ctx.commandList.update(_transformBuffer.get(), span{&trans, 1}.as_bytes());

    _material->bindMaterialToRender(ctx);
    _mesh->bindVertexBuffers(ctx);
    ctx.commandList.bindConstantBuffer(2, _transformBuffer.get(), GpuShaderStage::All);
    ctx.commandList.setPrimitiveTopology(GpuPrimitiveTopology::Triangles);
    ctx.commandList.drawIndexed(static_cast<uint32>(_mesh->indexCount()));
}
