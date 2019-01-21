// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "D3D11_pipeline_state.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/math/packed.h"

gm::PipelineStateD3D11::PipelineStateD3D11(com_ptr<ID3D11RasterizerState> rasterState, com_ptr<ID3D11DepthStencilState> depthStencilState, com_ptr<ID3D11BlendState> blendState, com_ptr<ID3D11InputLayout> inputLayout)
    : _rasterState(std::move(rasterState)),
      _depthStencilState(std::move(depthStencilState)),
      _blendState(std::move(blendState)),
      _inputLayout(std::move(inputLayout)) {
    GM_ASSERT(_rasterState != nullptr);
    GM_ASSERT(_depthStencilState != nullptr);
    GM_ASSERT(_blendState != nullptr);
}

gm::PipelineStateD3D11::~PipelineStateD3D11() = default;

auto gm::PipelineStateD3D11::createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D11Device* device) -> box<PipelineStateD3D11> {
    GM_ASSERT(device != nullptr);

    D3D11_RASTERIZER_DESC rasterDesc = {};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};

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
    layout.AlignedByteOffset = 0;
    layout.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layout.InputSlot = 0;
    layout.SemanticName = "POSITION";

    com_ptr<ID3D11RasterizerState> rasterState;
    com_ptr<ID3D11DepthStencilState> depthStencilState;
    com_ptr<ID3D11BlendState> blendState;
    com_ptr<ID3D11InputLayout> inputLayout;

    HRESULT hr = device->CreateRasterizerState(&rasterDesc, out_ptr(rasterState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateDepthStencilState(&depthStencilDesc, out_ptr(depthStencilState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateBlendState(&blendDesc, out_ptr(blendState));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    hr = device->CreateInputLayout(&layout, 1, desc.vertShader.data(), desc.vertShader.size(), out_ptr(inputLayout));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    //pipeDesc.PrimitiveTopologyType = D3D11_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    //pipeDesc.InputLayout.NumElements = 1;
    //pipeDesc.InputLayout.pInputElementDescs = &layout;
    //pipeDesc.NumRenderTargets = 1;
    //pipeDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    //pipeDesc.SampleDesc.Count = 1;
    //pipeDesc.SampleDesc.Quality = 0;
    //pipeDesc.VS.pShaderBytecode = desc.vertShader.data();
    //pipeDesc.VS.BytecodeLength = desc.vertShader.size();

    return make_box<PipelineStateD3D11>(std::move(rasterState), std::move(depthStencilState), std::move(blendState), std::move(inputLayout));
}

void gm::PipelineStateD3D11::apply(ID3D11DeviceContext* context) const {
    GM_ASSERT(context != nullptr);

    context->IASetInputLayout(_inputLayout.get());
    context->RSSetState(_rasterState.get());
    context->OMSetDepthStencilState(_depthStencilState.get(), 0);

    PackedVector4f blend{0, 0, 0, 0};
    context->OMSetBlendState(_blendState.get(), blend, 0);
}
