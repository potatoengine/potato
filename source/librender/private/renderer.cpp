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
#include "gpu_renderable.h"
#include "material.h"
#include "mesh.h"
#include "shader.h"
#include "texture.h"

#include "potato/runtime/asset_loader.h"
#include "potato/runtime/stream.h"

#include <chrono>

constexpr int debug_vbo_size = 64 * 1024;
constexpr double nano_to_seconds = 1.0 / 1000000000.0;

up::Renderer::Renderer(rc<GpuDevice> device) : _device(std::move(device)) {

    //_debugLineMaterial = _loader.loadMaterialSync("materials/debug_line.mat");
    //_debugLineBuffer = _device->createBuffer(GpuBufferType::Vertex, debug_vbo_size);

    //// Create the debug pipeline
    //GpuPipelineStateDesc pipelineDesc;

    //GpuInputLayoutElement const layout[] = {
    //    {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
    //    {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
    //    {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
    //    {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
    //    {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
    //};

    //pipelineDesc.enableDepthTest = true;
    //pipelineDesc.enableDepthWrite = true;
    //pipelineDesc.vertShader = _device->getDebugShader(GpuShaderStage::Vertex).as_bytes();
    //pipelineDesc.pixelShader = _device->getDebugShader(GpuShaderStage::Pixel).as_bytes();
    //pipelineDesc.inputLayout = layout;

    //// Check to support null renderer; should this be explicit?
    //if (!pipelineDesc.vertShader.empty() && !pipelineDesc.pixelShader.empty()) {
    //    _debugState = _device->createPipelineState(pipelineDesc);
    //}
}

up::Renderer::~Renderer() = default;

void up::Renderer::beginFrame(GpuSwapChain* swapChain) {
    constexpr double nanoToSeconds = 1.0 / 1000000000.0;
    UP_ASSERT(swapChain != nullptr);
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

    _device->beginFrame(swapChain);
    for (auto& renderable : _rendarables) {
        _device->render(frame, renderable.get());
    }
    _rendarables.clear(); 
    _device->endFrame(swapChain);
}

void up::Renderer::endFrame(GpuSwapChain* swapChain, float frameTime) {
    UP_ASSERT(swapChain != nullptr);
    _device->execute(false);
}

void up::Renderer::quit() {
    _device->execute(true);
}


auto up::Renderer::createRendarable(IRenderable* pInterface) -> GpuRenderable* {

    auto renderable = _device->createRenderable(pInterface);
    return _rendarables.emplace_back(std::move(renderable)).get();
}

namespace up {
    namespace {
        class MeshAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Mesh::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Mesh::createFromBuffer(ctx.key, contents);
            }
        };

        class MaterialAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Material::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return Material::createFromBuffer(ctx.key, contents, ctx.loader);
            }
        };

        class ShaderAssetLoaderBackend : public AssetLoaderBackend {
        public:
            zstring_view typeName() const noexcept override { return Shader::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                vector<byte> contents;
                if (auto rs = readBinary(ctx.stream, contents); rs != IOResult::Success) {
                    return nullptr;
                }
                ctx.stream.close();

                return up::new_shared<Shader>(ctx.key, std::move(contents));
            }
        };

        class TextureAssetLoaderBackend : public AssetLoaderBackend {
        public:
            TextureAssetLoaderBackend(Renderer& renderer) : _renderer(renderer) {}

            zstring_view typeName() const noexcept override { return Texture::assetTypeName; }
            rc<Asset> loadFromStream(AssetLoadContext const& ctx) override {
                auto img = loadImage(ctx.stream);
                ctx.stream.close();
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

                return new_shared<Texture>(ctx.key, std::move(img), std::move(tex));
            }

        private:
            Renderer& _renderer;
        };
    } // namespace
} // namespace up

void up::Renderer::registerAssetBackends(AssetLoader& assetLoader) {
    UP_ASSERT(_device != nullptr);
    assetLoader.registerBackend(new_box<MeshAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<MaterialAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<ShaderAssetLoaderBackend>());
    assetLoader.registerBackend(new_box<TextureAssetLoaderBackend>(*this));
    _device->registerAssetBackends(assetLoader);
}

void up::Renderer::clearCommandList() {
    _device->clearCommandList();
}
