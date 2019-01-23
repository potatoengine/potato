// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/grui/grui.h"
#include "grimm/gpu/buffer.h"
#include "grimm/gpu/command_list.h"
#include "grimm/gpu/device.h"
#include "grimm/gpu/pipeline_state.h"
#include "grimm/gpu/texture.h"
#include "grimm/gpu/resource_view.h"
#include "grimm/gpu/sampler.h"
#include <imgui.h>

static constexpr gm::uint32 bufferSize = 64 * 1024;

gm::gui::DrawImgui::DrawImgui() = default;
gm::gui::DrawImgui::~DrawImgui() = default;

bool gm::gui::DrawImgui::createResources(gpu::Device& device, ImGuiIO const& imguiIO, blob vertShader, blob pixelShader) {
    gpu::InputLayoutElement layout[] = {
        {gpu::Format::R32G32Float, gpu::Semantic::Position, 0, 0},
        {gpu::Format::R32G32Float, gpu::Semantic::TexCoord, 0, 0},
        {gpu::Format::R8G8B8A8UnsignedNormalized, gpu::Semantic::Color, 0, 0},
    };

    gpu::PipelineStateDesc desc;
    desc.vertShader = std::move(vertShader);
    desc.pixelShader = std::move(pixelShader);
    desc.inputLayout = layout;

    _indexBuffer = device.createBuffer(gpu::BufferType::Index, bufferSize);
    _vertexBuffer = device.createBuffer(gpu::BufferType::Vertex, bufferSize);
    _constantBuffer = device.createBuffer(gpu::BufferType::Constant, sizeof(float) * 16);
    _pipelineState = device.createPipelineState(desc);

    int fontWidth, fontHeight;
    unsigned char* pixels;
    imguiIO.Fonts->GetTexDataAsRGBA32(&pixels, &fontWidth, &fontHeight);
    auto font = device.createTexture2D(fontWidth, fontHeight, gpu::Format::R8G8B8A8UnsignedNormalized, span{pixels, static_cast<uint32>(fontWidth * fontHeight * 4)}.as_bytes());
    _srv = device.createShaderResourceView(font.get());

    _sampler = device.createSampler();

    return true;
}

void gm::gui::DrawImgui::releaseResources() {
    _indexBuffer.reset();
    _vertexBuffer.reset();
    _constantBuffer.reset();
    _pipelineState.reset();
    _srv.reset();
    _sampler.reset();
}

void gm::gui::DrawImgui::draw(ImDrawData const& data, gpu::CommandList& commandList) {
    GM_ASSERT(data.Valid, "DrawImgui::draw() can only be called after Render() but before BeginFrame()");

    GM_ASSERT(data.TotalIdxCount * sizeof(ImDrawIdx) <= bufferSize, "Too many ImGui indices");
    GM_ASSERT(data.TotalVtxCount * sizeof(ImDrawVert) <= bufferSize, "Too many ImGui verticies");

    commandList.setPipelineState(_pipelineState.get());
    commandList.setPrimitiveTopology(gpu::PrimitiveTopology::Triangles);

    auto indices = commandList.map(_indexBuffer.get(), bufferSize);
    auto vertices = commandList.map(_vertexBuffer.get(), bufferSize);

    for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
        auto const& list = *data.CmdLists[listIndex];

        std::memcpy(indices.data(), list.IdxBuffer.Data, list.IdxBuffer.Size * sizeof(ImDrawIdx));
        std::memcpy(indices.data(), list.VtxBuffer.Data, list.VtxBuffer.Size * sizeof(ImDrawVert));
    }

    commandList.unmap(_indexBuffer.get(), indices);
    commandList.unmap(_vertexBuffer.get(), vertices);

    float L = data.DisplayPos.x;
    float R = data.DisplayPos.x + data.DisplaySize.x;
    float T = data.DisplayPos.y;
    float B = data.DisplayPos.y + data.DisplaySize.y;
    float mvp[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, 0.5f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f},
    };

    auto constants = commandList.map(_constantBuffer.get(), sizeof(mvp));
    std::memcpy(constants.data(), mvp, constants.size());
    commandList.unmap(_constantBuffer.get(), constants);

    commandList.bindIndexBuffer(_indexBuffer.get(), gpu::IndexType::Unsigned16, 0);
    commandList.bindVertexBuffer(0, _vertexBuffer.get(), sizeof(ImDrawVert));
    commandList.bindConstantBuffer(0, _constantBuffer.get(), gpu::ShaderStage::Vertex);
    commandList.bindShaderResource(0, _srv.get(), gpu::ShaderStage::Pixel);
    commandList.bindSampler(0, _sampler.get(), gpu::ShaderStage::Pixel);

    gpu::Viewport viewport;
    viewport.height = data.DisplaySize.x;
    viewport.width = data.DisplaySize.y;
    viewport.leftX = data.DisplayPos.x;
    viewport.topY = data.DisplayPos.y;
    commandList.setViewport(viewport);

    uint32 indexOffset = 0;

    for (int listIndex = 0; listIndex != data.CmdListsCount; ++listIndex) {
        auto const& list = *data.CmdLists[listIndex];

        for (int cmdIndex = 0; cmdIndex != list.CmdBuffer.Size; ++cmdIndex) {
            auto const& cmd = list.CmdBuffer.Data[cmdIndex];

            if (cmd.UserCallback != nullptr) {
                cmd.UserCallback(&list, &cmd);
                continue;
            }

            commandList.drawIndexed(cmd.ElemCount, indexOffset);
            indexOffset += cmd.ElemCount;
        }
    }
}
