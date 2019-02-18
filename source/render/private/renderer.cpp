// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "renderer.h"
#include "render_task.h"
#include "context.h"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "debug_draw.h"
#include <grimm/gpu/buffer.h>
#include <grimm/gpu/command_list.h>
#include <grimm/gpu/device.h>
#include <grimm/gpu/swap_chain.h>
#include <grimm/gpu/texture.h>
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/stream_util.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <chrono>

namespace {
    struct FrameData {
        gm::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };
} // namespace

gm::Renderer::Renderer(fs::FileSystem fileSystem, rc<gpu::Device> device) : _device(std::move(device)), _fileSystem(std::move(fileSystem)), _renderThread([this] { _renderMain(); }) {
    _commandList = _device->createCommandList();

    _debugLineMaterial = loadMaterialSync("resources/materials/debug_line.json");
    _debugLineBuffer = _device->createBuffer(gpu::BufferType::Vertex, 64 * 1024);
}

gm::Renderer::~Renderer() {
    _taskQueue.close();
    _renderThread.join();
}

void gm::Renderer::_renderMain() {
    RenderTask task;
    while (_taskQueue.dequeWait(task)) {
        task();
    }
}

void gm::Renderer::beginFrame() {
    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(gpu::BufferType::Constant, sizeof(FrameData));
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
    _commandList->bindConstantBuffer(0, _frameDataBuffer.get(), gpu::ShaderStage::All);
}

void gm::Renderer::endFrame(float frameTime) {
    auto ctx = context();
    _debugLineMaterial->bindMaterialToRender(ctx);
    flushDebugDraw(*_device, *_commandList, *_debugLineBuffer, frameTime);
    _commandList->finish();
    _device->execute(_commandList.get());
}

auto gm::Renderer::context() -> RenderContext {
    return RenderContext{
        _frameTimestamp,
        *_commandList,
        *_device};
}

auto gm::Renderer::loadMeshSync(zstring_view path) -> rc<Mesh> {
    blob contents;
    auto stream = _fileSystem.openRead(path);
    if (fs::readBlob(stream, contents) != fs::Result{}) {
        return {};
    }
    stream.close();

    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFileFromMemory(contents.data(), contents.size(), aiProcess_FlipWindingOrder, "assbin");
    if (scene == nullptr) {
        zstring_view error = importer.GetErrorString();
        return {};
    }
    aiMesh const* mesh = scene->mMeshes[0];

    MeshChannel channels[] = {
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Position},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::Color},
        {0, gpu::Format::R32G32B32Float, gpu::Semantic::TexCoord},
    };

    uint16 stride = sizeof(float) * 8;
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
        data.push_back(mesh->mTextureCoords[0][i].x);
        data.push_back(mesh->mTextureCoords[0][i].y);
    }

    return make_shared<Mesh>(std::move(indices), blob(span{data.data(), data.size()}.as_bytes()), span{&bufferDesc, 1}, channels);
}

auto gm::Renderer::loadMaterialSync(zstring_view path) -> rc<Material> {
    std::ifstream inFile(path.c_str());
    if (!inFile) {
        return nullptr;
    }

    rapidjson::Document doc;
    rapidjson::IStreamWrapper inStream(inFile);
    doc.ParseStream<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(inStream);
    if (doc.HasParseError()) {
        return nullptr;
    }

    inFile.close();

    rc<Shader> vertex;
    rc<Shader> pixel;
    vector<rc<Texture>> textures;

    auto root = doc.GetObject();
    auto& shaders = root["shaders"];
    if (shaders.IsObject()) {
        auto& vertexPath = shaders["vertex"];
        auto& pixelPath = shaders["pixel"];

        if (vertexPath.IsString()) {
            vertex = loadShaderSync(vertexPath.GetString());
        }

        if (pixelPath.IsString()) {
            pixel = loadShaderSync(pixelPath.GetString());
        }
    }

    auto& texts = root["textures"];
    if (texts.IsArray()) {
        for (auto& texPath : texts.GetArray()) {
            if (!texPath.IsString()) {
                return nullptr;
            }

            auto tex = loadTextureSync(texPath.GetString());
            if (!tex) {
                return nullptr;
            }

            textures.push_back(std::move(tex));
        }
    }

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
    }

    return make_shared<Material>(std::move(vertex), std::move(pixel), std::move(textures));
}

auto gm::Renderer::loadShaderSync(zstring_view path) -> rc<Shader> {
    blob contents;
    auto stream = _fileSystem.openRead(path);
    if (fs::readBlob(stream, contents) != fs::Result{}) {
        return {};
    }
    return make_shared<Shader>(std::move(contents));
}

auto gm::Renderer::loadTextureSync(zstring_view path) -> rc<Texture> {
    fs::Stream stream = _fileSystem.openRead(path);
    if (!stream) {
        return nullptr;
    }

    auto img = image::loadImage(stream);
    if (img.size() == 0) {
        return nullptr;
    }

    gpu::TextureDesc desc = {};
    desc.type = gpu::TextureType::Texture2D;
    desc.format = gpu::Format::R8G8B8A8UnsignedNormalized;
    desc.width = img.width();
    desc.height = img.height();

    auto tex = _device->createTexture2D(desc, {img.data(), img.size()});
    if (tex == nullptr) {
        return nullptr;
    }

    return make_shared<Texture>(std::move(img), std::move(tex));
}
