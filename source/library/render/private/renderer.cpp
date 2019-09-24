// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/spud/numeric_util.h"
#include "potato/render/renderer.h"
#include "potato/render/render_task.h"
#include "potato/render/context.h"
#include "potato/render/material.h"
#include "potato/render/mesh.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/render/debug_draw.h"
#include "potato/gpu/buffer.h"
#include "potato/gpu/command_list.h"
#include "potato/gpu/device.h"
#include "potato/gpu/swap_chain.h"
#include "potato/gpu/texture.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/json.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <chrono>

namespace {
    struct FrameData {
        up::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

up::Renderer::Renderer(FileSystem& fileSystem, rc<GpuDevice> device) : _device(std::move(device)), _fileSystem(fileSystem), _renderThread([this] { _renderMain(); }) {
    _commandList = _device->createCommandList();

    _debugLineMaterial = loadMaterialSync("resources/materials/debug_line.json");
    _debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, 64 * 1024);
}

up::Renderer::~Renderer() {
    _taskQueue.close();
    _renderThread.join();
}

void up::Renderer::_renderMain() {
    RenderTask task;
    while (_taskQueue.dequeWait(task)) {
        task();
    }
}

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

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(contents.data(), contents.size(), 0, "assbin");
    if (scene == nullptr) {
        // FIXME: how to report this?
        //zstring_view error = importer.GetErrorString();
        return {};
    }
    aiMesh const* mesh = scene->mMeshes[0];

    MeshChannel channels[] = {
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Position},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Color},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal},
        {0, GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent},
        {0, GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord},
    };

    uint16 stride = sizeof(float) * 14;
    uint32 size = mesh->mNumVertices * stride;

    MeshBuffer bufferDesc = {size, 0, stride};

    vector<uint16> indices;
    indices.reserve(mesh->mNumFaces * 3);

    vector<float> data;
    data.reserve(mesh->mNumVertices);

    for (uint32 i = 0; i != mesh->mNumFaces; ++i) {
        indices.push_back(mesh->mFaces[i].mIndices[0]);
        indices.push_back(mesh->mFaces[i].mIndices[1]);
        indices.push_back(mesh->mFaces[i].mIndices[2]);
    }

    for (uint32 i = 0; i != mesh->mNumVertices; ++i) {
        data.push_back(mesh->mVertices[i].x);
        data.push_back(mesh->mVertices[i].y);
        data.push_back(mesh->mVertices[i].z);

        if (mesh->GetNumColorChannels() >= 1) {
            data.push_back(mesh->mColors[0][i].r);
            data.push_back(mesh->mColors[0][i].g);
            data.push_back(mesh->mColors[0][i].b);
        }
        else {
            data.push_back(1.f);
            data.push_back(1.f);
            data.push_back(1.f);
        }

        if (mesh->HasNormals()) {
            data.push_back(mesh->mNormals[i].x);
            data.push_back(mesh->mNormals[i].y);
            data.push_back(mesh->mNormals[i].z);
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
            data.push_back(0.f);
        }

        if (mesh->HasTangentsAndBitangents()) {
            data.push_back(mesh->mTangents[i].x);
            data.push_back(mesh->mTangents[i].y);
            data.push_back(mesh->mTangents[i].z);
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
            data.push_back(0.f);
        }

        if (mesh->HasTextureCoords(0)) {
            data.push_back(mesh->mTextureCoords[0][i].x);
            data.push_back(mesh->mTextureCoords[0][i].y);
        }
        else {
            data.push_back(0.f);
            data.push_back(0.f);
        }
    }

    return up::new_shared<Mesh>(std::move(indices), vector(data.as_bytes()), span{&bufferDesc, 1}, channels);
}

auto up::Renderer::loadMaterialSync(zstring_view path) -> rc<Material> {
    std::ifstream inFile(path.c_str());
    if (!inFile) {
        return nullptr;
    }

    auto jsonRoot = nlohmann::json::parse(inFile);
    inFile.close();

    rc<Shader> vertex;
    rc<Shader> pixel;
    vector<rc<Texture>> textures;

    auto jsonShaders = jsonRoot["shaders"];
    if (jsonShaders.is_object()) {
        auto vertexPath = jsonShaders["vertex"].get<string>();
        auto pixelPath = jsonShaders["pixel"].get<string>();

        vertex = loadShaderSync(vertexPath);
        pixel = loadShaderSync(pixelPath);
    }

    auto jsonTextures = jsonRoot["textures"];
    for (auto jsonTexture : jsonTextures) {
        auto texturePath = jsonTexture.get<string>();

        auto tex = loadTextureSync(texturePath);
        if (!tex) {
            return nullptr;
        }

        textures.push_back(std::move(tex));
    }

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
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
