// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "renderer.h"
#include "context.h"
#include "debug_draw.h"
#include "gpu_buffer.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_pipeline_state.h"
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
} // namespace

up::Renderer::Renderer(rc<GpuDevice> device) : _device(std::move(device)) {
    _commandList = _device->createCommandList();

    // Create the debug pipeline
    GpuPipelineStateDesc pipelineDesc;

    GpuInputLayoutElement const layout[] = {
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
        {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
        {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
    };

    pipelineDesc.enableDepthTest = true;
    pipelineDesc.enableDepthWrite = true;
    pipelineDesc.vertShader = _device->getDebugShader(GpuShaderStage::Vertex).as_bytes();
    pipelineDesc.pixelShader = _device->getDebugShader(GpuShaderStage::Pixel).as_bytes();
    pipelineDesc.inputLayout = layout;

    // Check to support null renderer; should this be explicit?
    if (!pipelineDesc.vertShader.empty() && !pipelineDesc.pixelShader.empty()) {
        _debugState = _device->createPipelineState(pipelineDesc);
    }
}

up::Renderer::~Renderer() = default;

void up::Renderer::beginFrame() {
    constexpr double nanoToSeconds = 1.0 / 1000000000.0;

    if (_frameDataBuffer == nullptr) {
        _frameDataBuffer = _device->createBuffer(GpuBufferType::Constant, sizeof(FrameData));
    }

    uint64 nowNanoseconds = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    if (_startTimestamp == 0) {
        _startTimestamp = nowNanoseconds;
    }

    double const now = static_cast<double>(nowNanoseconds - _startTimestamp) * nanoToSeconds;
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

    if (_debugState.empty()) {
        up::flushDebugDraw(frameTime);
        return;
    }

    if (_debugBuffer == nullptr) {
        _debugBuffer = _device->createBuffer(GpuBufferType::Vertex, bufferSize);
    }

    _commandList->setPipelineState(_debugState.get());
    _commandList->bindVertexBuffer(0, _debugBuffer.get(), sizeof(DebugDrawVertex));
    _commandList->setPrimitiveTopology(GpuPrimitiveTopology::Lines);

    dumpDebugDraw([this](auto debugVertices) {
        if (debugVertices.empty()) {
            return;
        }

        uint32 vertCount = min(static_cast<uint32>(debugVertices.size()), maxVertsPerChunk);
        uint32 offset = 0;
        while (offset < debugVertices.size()) {
            _commandList->update(_debugBuffer.get(), debugVertices.subspan(offset, vertCount).as_bytes());
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

namespace up {
    namespace {
        class MeshResourceLoaderBackend : public ResourceLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Mesh::resourceType; }
            rc<Resource> loadFromStream(Stream stream, ResourceLoader& resourceLoader) override {
                vector<byte> contents;
                if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                stream.close();

                return Mesh::createFromBuffer(contents);
            }
        };

        class MaterialResourceLoaderBackend : public ResourceLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Material::resourceType; }
            rc<Resource> loadFromStream(Stream stream, ResourceLoader& resourceLoader) override {
                vector<byte> contents;
                if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                stream.close();

                return Material::createFromBuffer(contents, resourceLoader);
            }
        };

        class ShaderResourceLoaderBackend : public ResourceLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Shader::resourceType; }
            rc<Resource> loadFromStream(Stream stream, ResourceLoader& resourceLoader) override {
                vector<byte> contents;
                if (auto rs = readBinary(stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                stream.close();

                return up::new_shared<Shader>(std::move(contents));
            }
        };

        class TextureResourceLoaderBackend : public ResourceLoaderBackend {
        public:
            TextureResourceLoaderBackend(Renderer& renderer) : _renderer(renderer) {}

            zstring_view typeName() const noexcept override { return Texture::resourceType; }
            rc<Resource> loadFromStream(Stream stream, ResourceLoader& resourceLoader) override {
                auto img = loadImage(stream);
                if (img.data().empty()) {
                    return nullptr;
                }

                GpuTextureDesc desc = {};
                desc.type = GpuTextureType::Texture2D;
                desc.format = GpuFormat::R8G8B8A8UnsignedNormalized;
                desc.width = img.header().width;
                desc.height = img.header().height;

                auto tex = _renderer.device().createTexture2D(desc, img.data());
                if (tex == nullptr) {
                    return nullptr;
                }

                return new_shared<Texture>(std::move(img), std::move(tex));
            }

        private:
            Renderer& _renderer;
        };
    } // namespace
} // namespace up

void up::Renderer::registerResourceBackends(ResourceLoader& resourceLoader) {
    UP_ASSERT(_device != nullptr);
    resourceLoader.addBackend(new_box<MeshResourceLoaderBackend>());
    resourceLoader.addBackend(new_box<MaterialResourceLoaderBackend>());
    resourceLoader.addBackend(new_box<ShaderResourceLoaderBackend>());
    resourceLoader.addBackend(new_box<TextureResourceLoaderBackend>(*this));
    _device->registerResourceBackends(resourceLoader);
}

up::DefaultLoader::DefaultLoader(ResourceLoader& resourceLoader, rc<GpuDevice> device)
    : _resourceLoader(resourceLoader)
    , _device(std::move(device)) {}

up::DefaultLoader::~DefaultLoader() = default;
