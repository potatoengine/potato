// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/material.h"
#include "potato/render/context.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/gpu/command_list.h"
#include "potato/gpu/pipeline_state.h"
#include "potato/gpu/resource_view.h"
#include "potato/gpu/device.h"
#include "potato/gpu/sampler.h"

up::Material::Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<GpuTexture>> textures) : _vertexShader(std::move(vertexShader)), _pixelShader(std::move(pixelShader)), _textures(std::move(textures)), _srvs(_textures.size()), _samplers(_textures.size()) {}

up::Material::~Material() = default;

void up::Material::bindMaterialToRender(RenderContext& ctx) {
    if (_state == nullptr) {
        gpu::PipelineStateDesc pipelineDesc;

        gpu::InputLayoutElement layout[] = {
            {gpu::Format::R32G32B32Float, gpu::Semantic::Position, 0, 0},
            {gpu::Format::R32G32B32Float, gpu::Semantic::Color, 0, 0},
            {gpu::Format::R32G32Float, gpu::Semantic::TexCoord, 0, 0},
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
        ctx.commandList.bindSampler(texIndex, _samplers[texIndex].get(), gpu::ShaderStage::Pixel);
        ctx.commandList.bindShaderResource(texIndex++, srv.get(), gpu::ShaderStage::Pixel);
    }
}
