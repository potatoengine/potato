// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "material.h"
#include "context.h"
#include "shader.h"
#include "texture.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/pipeline_state.h"
#include "grimm/gpu/resource_view.h"
#include "grimm/gpu/device.h"

gm::Material::Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures) : _vertexShader(std::move(vertexShader)), _pixelShader(std::move(pixelShader)), _textures(std::move(textures)), _srvs(_textures.size()) {}

gm::Material::~Material() = default;

void gm::Material::bindMaterialToRender(RenderContext& ctx) {
    if (_state == nullptr) {
        gpu::PipelineStateDesc pipelineDesc;

        gpu::InputLayoutElement layout[2] = {
            {gpu::Format::R32G32B32Float, gpu::Semantic::Position, 0, 0},
            {gpu::Format::R32G32B32Float, gpu::Semantic::Color, 0, 0},
        };

        pipelineDesc.enableDepthTest = true;
        pipelineDesc.enableDepthWrite = true;
        pipelineDesc.vertShader = _vertexShader->content();
        pipelineDesc.pixelShader = _pixelShader->content();
        pipelineDesc.inputLayout = layout;
        _state = ctx.device.createPipelineState(pipelineDesc);

        int texIndex = 0;
        for (auto const& tex : _textures) {
            _srvs[texIndex++] = ctx.device.createShaderResourceView(&tex->texture());
        }
    }

    ctx.commandList.setPipelineState(_state.get());

    int texIndex = 0;
    for (auto const& srv : _srvs) {
        ctx.commandList.bindShaderResource(texIndex++, srv.get(), gpu::ShaderStage::Pixel);
    }
}
