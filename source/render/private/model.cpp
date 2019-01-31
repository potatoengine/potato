// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "model.h"
#include "material.h"
#include "mesh.h"
#include "context.h"
#include "grimm/math/packed_op.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/command_list.h"

namespace {
    struct alignas(16) Vert {
        gm::Packed3 pos;
        gm::Packed3 color;
    };

    struct alignas(16) Trans {
        float modelWorld[16];
        float worldModel[26];
    };
} // namespace

static Vert cube[6 * 6];

static void makeFace(int index, gm::Packed3 normal, gm::Packed3 up, gm::Packed3 right) {
    cube[index++] = Vert{
        normal - right - up,
        abs(normal)};
    cube[index++] = Vert{
        normal + right - up,
        abs(normal)};
    cube[index++] = Vert{
        normal + right + up,
        abs(normal)};
    cube[index++] = Vert{
        normal - right - up,
        abs(normal)};
    cube[index++] = Vert{
        normal + right + up,
        abs(normal)};
    cube[index++] = Vert{
        normal - right + up,
        abs(normal)};
}
static void makeCube() {
    makeFace(0, {1, 0, 0}, {0, 1, 0}, {0, 0, 1});
    makeFace(6, {-1, 0, 0}, {0, 1, 0}, {0, 0, 1});
    makeFace(12, {0, 0, 1}, {0, 1, 0}, {1, 0, 0});
    makeFace(18, {0, 0, -1}, {0, 1, 0}, {-1, 0, 0});
    makeFace(24, {0, 1, 0}, {1, 0, 0}, {0, 0, 1});
    makeFace(30, {0, -1, 0}, {1, 0, 0}, {0, 0, 1});
}

gm::Model::Model(rc<Material> material) : _material(std::move(material)) {
    MeshBuffer buffer;
    buffer.stride = sizeof(Vert);

    MeshChannel channels[2] = {
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Position},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Color},
    };

    makeCube();
    _mesh = make_shared<Mesh>(blob(span{cube, std::size(cube)}.as_bytes()), span{&buffer, 1}, channels);
}

gm::Model::~Model() = default;

void GM_VECTORCALL gm::Model::render(RenderContext& ctx, Mat4x4 transform) {
    if (_transformBuffer == nullptr) {
        _transformBuffer = ctx.device.createBuffer(gpu::BufferType::Constant, sizeof(Trans));
    }

    Trans trans;
    transform.alignedStore(trans.modelWorld);
    transpose(transform).alignedStore(trans.worldModel);

    _mesh->updateVertexBuffers(ctx);
    ctx.commandList.update(_transformBuffer.get(), span{&trans, 1}.as_bytes());

    _material->bindMaterialToRender(ctx);
    _mesh->bindVertexBuffers(ctx);
    ctx.commandList.bindConstantBuffer(2, _transformBuffer.get(), gpu::ShaderStage::All);
    ctx.commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);
    ctx.commandList.draw(static_cast<uint32>(std::size(cube)));
}