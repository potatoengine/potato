// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/mesh.h"
#include "potato/render/context.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "model_generated.h"

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

auto up::Mesh::createFromBuffer(view<byte> buffer) -> rc<Mesh> {
    flatbuffers::Verifier verifier(reinterpret_cast<uint8 const*>(buffer.data()), buffer.size());
    if (!schema::VerifyModelBuffer(verifier)) {
        return {};
    }

    auto flatModel = schema::GetModel(buffer.data());
    if (flatModel == nullptr) {
        return {};
    }

    auto flatMeshes = flatModel->meshes();
    if (flatMeshes == nullptr) {
        return {};
    }
    if (flatMeshes->size() == 0) {
        return {};
    }

    auto flatMesh = flatModel->meshes()->Get(0);
    if (flatMesh == nullptr) {
        return {};
    }

    MeshChannel channels[] = {
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Position},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Color},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent},
        {0, GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord},
    };

    uint16 stride = sizeof(float) * 14;
    uint32 numVertices = flatMesh->vertices()->size();
    uint32 size = numVertices * stride;

    MeshBuffer bufferDesc = {size, 0, stride};

    vector<uint16> indices;
    indices.reserve(flatMesh->indices()->size());

    vector<float> data;
    data.reserve(numVertices);

    auto flatIndices = flatMesh->indices();
    auto flatVerts = flatMesh->vertices();
    auto flatColors = flatMesh->colors();
    auto flatNormals = flatMesh->normals();
    auto flatTangents = flatMesh->tangents();
    auto flatUVs = flatMesh->uvs();

    for (uint32 i = 0; i != flatIndices->size(); ++i) {
        indices.push_back(flatIndices->Get(i));
    }

    for (uint32 i = 0; i != numVertices; ++i) {
        auto vert = *flatVerts->Get(i);
        data.push_back(vert.x());
        data.push_back(vert.y());
        data.push_back(vert.z());

        if (flatColors != nullptr) {
            auto color = *flatColors->Get(i);
            data.push_back(color.x());
            data.push_back(color.y());
            data.push_back(color.z());
        }
        else {
            data.push_back(1.f);
            data.push_back(1.f);
            data.push_back(1.f);
        }

        if (flatNormals != nullptr) {
            auto norm = *flatNormals->Get(i);
            data.push_back(norm.x());
            data.push_back(norm.y());
            data.push_back(norm.z());
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
            data.push_back(0.f);
        }

        if (flatTangents != nullptr) {
            auto tangent = *flatTangents->Get(i);
            data.push_back(tangent.x());
            data.push_back(tangent.y());
            data.push_back(tangent.z());
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
            data.push_back(0.f);
        }

        if (flatUVs != nullptr) {
            auto tex = *flatUVs->Get(i);
            data.push_back(tex.x());
            data.push_back(tex.y());
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
        }
    }

    return up::new_shared<Mesh>(std::move(indices), vector(data.as_bytes()), span{&bufferDesc, 1}, channels);
}
