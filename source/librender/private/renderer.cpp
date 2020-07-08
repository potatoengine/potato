// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "renderer.h"
#include "context.h"
#include "debug_draw.h"
#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_swap_chain.h"
#include "gpu_texture.h"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

#include "potato/runtime/resource_loader.h"
#include "potato/runtime/stream.h"
#include "potato/spud/numeric_util.h"

#include <chrono>

namespace {
    struct FrameData {
        up::uint32 frameNumber = 0;
        float lastFrameTimeDelta = 0.f;
        double timeStamp = 0.0;
    };

    constexpr int debug_vbo_size = 64 * 1024;
    constexpr double nano_to_seconds = 1.0 / 1000000000.0;
} // namespace

up::Renderer::Renderer(Loader& loader, rc<GpuDevice> device) : _device(std::move(device)), _loader(loader) {
    _commandList = _device->createCommandList();

    _debugLineMaterial = _loader.loadMaterialSync("materials/debug_line.mat");
    _debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, debug_vbo_size);
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

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) * nano_to_seconds;
    FrameData frame = {_frameCounter++, static_cast<float>(now - _frameTimestamp), now};
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
    return RenderContext{_frameTimestamp, *_commandList, *_device};
}

up::DefaultLoader::DefaultLoader(ResourceLoader& resourceLoader, rc<GpuDevice> device)
    : _resourceLoader(resourceLoader)
    , _device(std::move(device)) {}

up::DefaultLoader::~DefaultLoader() = default;

auto up::DefaultLoader::loadMeshSync(zstring_view path) -> rc<Mesh> {
    vector<byte> contents;
    auto stream = _resourceLoader.openAsset(path);
    if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
        return nullptr;
    }
    stream.close();

    return Mesh::createFromBuffer(contents);
}

auto up::DefaultLoader::loadMaterialSync(zstring_view path) -> rc<Material> {
    vector<byte> contents;
    auto stream = _resourceLoader.openAsset(path);
    if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
        return nullptr;
    }
    stream.close();

    return Material::createFromBuffer(contents, *this);
}

auto up::DefaultLoader::loadShaderSync(zstring_view path, string_view logicalName) -> rc<Shader> {
    vector<byte> contents;
    auto stream = _resourceLoader.openAsset(path, logicalName);
    if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
        return nullptr;
    }
    stream.close();

    return up::new_shared<Shader>(std::move(contents));
}

auto up::DefaultLoader::loadTextureSync(zstring_view path) -> rc<Texture> {
    Stream stream = _resourceLoader.openAsset(path);
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
