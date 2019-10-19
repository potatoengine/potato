// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/spud/numeric_util.h"
#include "potato/render/renderer.h"
#include "potato/render/context.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/render/debug_draw.h"
#include "potato/render/gpu_buffer.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_swap_chain.h"
#include "potato/render/gpu_texture.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include "material_generated.h"
#include "model_generated.h"
#include <chrono>

namespace {
    struct FrameData {
        up::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

up::Renderer::Renderer(FileSystem& fileSystem, rc<GpuDevice> device) : _device(std::move(device)), _fileSystem(fileSystem) {
    _commandList = _device->createCommandList();

    _debugLineMaterial = loadMaterialSync("resources/materials/debug_line.mat");
    _debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, 64 * 1024);
}

up::Renderer::~Renderer() = default;

void up::Renderer::beginFrame() {
    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(GpuBufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) / 1000000000.0;
    FrameData frame = {
        _frameCounter++,
        static_cast<float>(now - _frameTimestamp),
        now};
    _frameTimestamp = now;

    _commandList->clear();
    _commandList->update(_frameDataBuffer.get(), view<byte>{reinterpret_cast<byte*>(&frame), sizeof(frame)});
    _commandList->bindConstantBuffer(0, _frameDataBuffer.get(), GpuShaderStage::All);
}

void up::Renderer::endFrame(float frameTime) {
    _commandList->finish();
    _device->execute(_commandList.get());
}

void up::Renderer::flushDebugDraw(float frameTime) {
    static constexpr uint32 bufferSize = 64 * 1024;
    static constexpr uint32 maxVertsPerChunk = bufferSize / sizeof(DebugDrawVertex);

    if (_debugLineBuffer == nullptr) {
        _debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, bufferSize);
    }

    auto ctx = context();
    _debugLineMaterial->bindMaterialToRender(ctx);
    _commandList->bindVertexBuffer(0, _debugLineBuffer.get(), sizeof(DebugDrawVertex));
    _commandList->setPrimitiveTopology(GpuPrimitiveTopology::Lines);

    dumpDebugDraw([this](auto debugVertices) {
        if (debugVertices.empty()) {
            return;
        }

        uint32 vertCount = min(static_cast<uint32>(debugVertices.size()), maxVertsPerChunk);
        uint32 offset = 0;
        while (offset < debugVertices.size()) {
            _commandList->update(_debugLineBuffer.get(), debugVertices.subspan(offset, vertCount).as_bytes());
            _commandList->draw(vertCount);

            offset += vertCount;
            vertCount = min(static_cast<uint32>(debugVertices.size()) - offset, maxVertsPerChunk);
        }
    });

    up::flushDebugDraw(frameTime);
}

auto up::Renderer::context() -> RenderContext {
    return RenderContext{
        _frameTimestamp,
        *_commandList,
        *_device};
}

auto up::Renderer::loadMeshSync(zstring_view path) -> rc<Mesh> {
    vector<byte> contents;
    auto stream = _fileSystem.openRead(path);
    if (readBinary(stream, contents) != IOResult{}) {
        return {};
    }
    stream.close();

    auto flatModel = schema::GetModel(contents.data());
    if (!flatModel) {
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
    if (!flatMesh) {
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

auto up::Renderer::loadMaterialSync(zstring_view path) -> rc<Material> {
    Stream input = _fileSystem.openRead(path);
    if (!input) {
        return nullptr;
    }

    vector<byte> data;
    if (auto rs = readBinary(input, data); rs != IOResult::Success) {
        return nullptr;
    }

    input.close();

    auto material = up::schema::GetMaterial(data.data());

    rc<Shader> vertex;
    rc<Shader> pixel;
    vector<rc<Texture>> textures;

    auto shader = material->shader();
    if (shader == nullptr) {
        return nullptr;
    }

    auto vertexPath = shader->vertex();
    auto pixelPath = shader->pixel();

    vertex = loadShaderSync(string(vertexPath->c_str(), vertexPath->size()));
    pixel = loadShaderSync(string(pixelPath->c_str(), pixelPath->size()));

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
    }

    for (auto textureData : *material->textures()) {
        auto texturePath = string(textureData->c_str(), textureData->size());

        auto tex = loadTextureSync(texturePath);
        if (!tex) {
            return nullptr;
        }

        textures.push_back(std::move(tex));
    }

    return new_shared<Material>(std::move(vertex), std::move(pixel), std::move(textures));
}

auto up::Renderer::loadShaderSync(zstring_view path) -> rc<Shader> {
    vector<byte> contents;
    auto stream = _fileSystem.openRead(path);
    if (readBinary(stream, contents) != IOResult{}) {
        return {};
    }
    return up::new_shared<Shader>(std::move(contents));
}

auto up::Renderer::loadTextureSync(zstring_view path) -> rc<Texture> {
    Stream stream = _fileSystem.openRead(path);
    if (!stream) {
        return nullptr;
    }

    auto img = loadImage(stream);
    if (img.data().empty()) {
        return nullptr;
    }

    GpuTextureDesc desc = {};
    desc.type = GpuTextureType::Texture2D;
    desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
    desc.width = img.header().width;
    desc.height = img.header().height;

    auto tex = _device->createTexture2D(desc, img.data());
    if (tex == nullptr) {
        return nullptr;
    }

    return new_shared<Texture>(std::move(img), std::move(tex));
}
