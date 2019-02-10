// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "material.h"
#include "context.h"
#include "shader.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/pipeline_state.h"
#include "grimm/gpu/device.h"

gm::Material::Material(rc<Shader> vertexShader, rc<Shader> pixelShader) : _vertexShader(std::move(vertexShader)), _pixelShader(std::move(pixelShader)) {}

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
    }

    ctx.commandList.setPipelineState(_state.get());
}
