// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "D3D11_pipeline_state.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"

gm::gpu::d3d11::PipelineStateD3D11::PipelineStateD3D11(PipelineStateParamsD3D11 params) : _params(std::move(params)) {
    GM_ASSERT(_params.rasterState != nullptr);
    GM_ASSERT(_params.depthStencilState != nullptr);
    GM_ASSERT(_params.blendState != nullptr);
    GM_ASSERT(_params.inputLayout != nullptr);
    GM_ASSERT(_params.vertShader != nullptr);
}

gm::gpu::d3d11::PipelineStateD3D11::~PipelineStateD3D11() = default;

auto gm::gpu::d3d11::PipelineStateD3D11::createGraphicsPipelineState(PipelineStateDesc const& desc, ID3D11Device* device) -> box<PipelineStateD3D11> {
    GM_ASSERT(device != nullptr);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.ScissorEnable = desc.enableScissor;
    rasterDesc.DepthClipEnable = true;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = desc.enableDepthTest;
    depthStencilDesc.DepthWriteMask = desc.enableDepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.AlphaToCoverageEnable = false;
    blendDesc.IndependentBlendEnable = false;
    blendDesc.RenderTarget[0].BlendEnable = true;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    D3D11_INPUT_ELEMENT_DESC layout[maxInputLayoutElements] = {};
    uint32 layoutIndex = 0;

    GM_ASSERT(desc.inputLayout.size() <= maxInputLayoutElements);

    for (InputLayoutElement const& element : desc.inputLayout) {
        D3D11_INPUT_ELEMENT_DESC& elemDesc = layout[layoutIndex++];
        elemDesc.SemanticName = toNative(element.semantic).c_str();
        elemDesc.SemanticIndex = element.semanticIndex;
        elemDesc.InputSlot = element.slot;
        elemDesc.Format = toNative(element.format);
        elemDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        elemDesc.InstanceDataStepRate = 0;
        elemDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
    }

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

    hr = device->CreateInputLayout(layout, layoutIndex, desc.vertShader.data(), desc.vertShader.size(), out_ptr(params.inputLayout));
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
