// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "renderer.h"
#include "render_task.h"
#include "context.h"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include <grimm/gpu/buffer.h>
#include <grimm/gpu/command_list.h>
#include <grimm/gpu/device.h>
#include <grimm/gpu/swap_chain.h>
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream.h"
#include "grimm/filesystem/stream_util.h"
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
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

void gm::Renderer::endFrame() {
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
    return nullptr;
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

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
    }

    return make_shared<Material>(std::move(vertex), std::move(pixel));
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
    return nullptr;
}
