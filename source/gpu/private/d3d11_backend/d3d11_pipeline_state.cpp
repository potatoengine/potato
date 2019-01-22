// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "D3D11_pipeline_state.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/math/packed.h"

gm::PipelineStateD3D11::PipelineStateD3D11(PipelineStateParamsD3D11 params) : _params(std::move(params)) {
    GM_ASSERT(_params.rasterState != nullptr);
    GM_ASSERT(_params.depthStencilState != nullptr);
    GM_ASSERT(_params.blendState != nullptr);
    GM_ASSERT(_params.inputLayout != nullptr);
    GM_ASSERT(_params.vertShader != nullptr);
}

gm::PipelineStateD3D11::~PipelineStateD3D11() = default;

auto gm::PipelineStateD3D11::createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D11Device* device) -> box<PipelineStateD3D11> {
    GM_ASSERT(device != nullptr);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = false;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_COLOR;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_INPUT_ELEMENT_DESC layout = {};
    layout.SemanticName = "POSITION";
    layout.SemanticIndex = 0;
    layout.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layout.InputSlot = 0;
    layout.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    layout.InstanceDataStepRate = 0;
    layout.AlignedByteOffset = 0;

    PipelineStateParamsD3D11 params;

    HRESULT hr = device->CreateRasterizerState(&rasterDesc, out_ptr(params.rasterState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateDepthStencilState(&depthStencilDesc, out_ptr(params.depthStencilState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateBlendState(&blendDesc, out_ptr(params.blendState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateInputLayout(&layout, 1, desc.vertShader.data(), desc.vertShader.size(), out_ptr(params.inputLayout));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateVertexShader(desc.vertShader.data(), desc.vertShader.size(), nullptr, out_ptr(params.vertShader));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreatePixelShader(desc.pixelShader.data(), desc.pixelShader.size(), nullptr, out_ptr(params.pixelShader));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return make_box<PipelineStateD3D11>(std::move(params));
}
