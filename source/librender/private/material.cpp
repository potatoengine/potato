// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "material.h"
#include "context.h"
#include "gpu_command_list.h"
#include "gpu_device.h"
#include "gpu_pipeline_state.h"
#include "gpu_resource_view.h"
#include "gpu_sampler.h"
#include "loader.h"
#include "material_generated.h"
#include "shader.h"
#include "texture.h"

#include "potato/spud/string.h"

up::Material::Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures)
    : _vertexShader(std::move(vertexShader))
    , _pixelShader(std::move(pixelShader))
    , _textures(std::move(textures))
    , _srvs(_textures.size())
    , _samplers(_textures.size()) {}

up::Material::~Material() = default;

void up::Material::bindMaterialToRender(RenderContext& ctx) {
    if (_state == nullptr) {
        GpuPipelineStateDesc pipelineDesc;

        GpuInputLayoutElement layout[] = {
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Position, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Color, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Normal, 0, 0},
            {GpuFormat::R32G32B32Float, GpuShaderSemantic::Tangent, 0, 0},
            {GpuFormat::R32G32Float, GpuShaderSemantic::TexCoord, 0, 0},
        };

        pipelineDesc.enableDepthTest = true;
        pipelineDesc.enableDepthWrite = true;
        pipelineDesc.vertShader = _vertexShader->content();
        pipelineDesc.pixelShader = _pixelShader->content();
        pipelineDesc.inputLayout = layout;
        _state = ctx.device.createPipelineState(pipelineDesc);

        int texIndex = 0;
        for (auto const& tex : _textures) {
            _srvs[texIndex] = ctx.device.createShaderResourceView(&tex->texture());
            _samplers[texIndex++] = ctx.device.createSampler();
        }
    }

    ctx.commandList.setPipelineState(_state.get());

    int texIndex = 0;
    for (auto const& srv : _srvs) {
        ctx.commandList.bindSampler(texIndex, _samplers[texIndex].get(), GpuShaderStage::Pixel);
        ctx.commandList.bindShaderResource(texIndex++, srv.get(), GpuShaderStage::Pixel);
    }
}

auto up::Material::createFromBuffer(view<byte> buffer, Loader& loader) -> rc<Material> {
    flatbuffers::Verifier verifier(reinterpret_cast<uint8 const*>(buffer.data()), buffer.size());
    if (!schema::VerifyMaterialBuffer(verifier)) {
        return {};
    }

    auto material = up::schema::GetMaterial(buffer.data());

    rc<Shader> vertex;
    rc<Shader> pixel;
    vector<rc<Texture>> textures;

    auto shader = material->shader();
    if (shader == nullptr) {
        return nullptr;
    }

    auto vertexPath = shader->vertex();
    auto pixelPath = shader->pixel();

    vertex = loader.loadShaderSync(string(vertexPath->c_str(), vertexPath->size()), "vertex"_sv);
    pixel = loader.loadShaderSync(string(pixelPath->c_str(), pixelPath->size()), "pixel"_sv);

    if (vertex == nullptr) {
        return nullptr;
    }

    if (pixel == nullptr) {
        return nullptr;
    }

    for (auto textureData : *material->textures()) {
        auto texturePath = string(textureData->c_str(), textureData->size());

        auto tex = loader.loadTextureSync(texturePath);
        if (!tex) {
            return nullptr;
        }

        textures.push_back(std::move(tex));
    }

    return new_shared<Material>(std::move(vertex), std::move(pixel), std::move(textures));
}
