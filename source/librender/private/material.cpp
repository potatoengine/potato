// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/render/material.h"
#include "potato/render/context.h"
#include "potato/render/shader.h"
#include "potato/render/texture.h"
#include "potato/render/gpu_command_list.h"
#include "potato/render/gpu_pipeline_state.h"
#include "potato/render/gpu_resource_view.h"
#include "potato/render/gpu_device.h"
#include "potato/render/gpu_sampler.h"

up::Material::Material(rc<Shader> vertexShader, rc<Shader> pixelShader, vector<rc<Texture>> textures) : _vertexShader(std::move(vertexShader)), _pixelShader(std::move(pixelShader)), _textures(std::move(textures)), _srvs(_textures.size()), _samplers(_textures.size()) {}

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
