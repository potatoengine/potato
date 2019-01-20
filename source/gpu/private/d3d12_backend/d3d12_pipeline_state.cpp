// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d12_pipeline_state.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/out_ptr.h"

gm::D3d12PipelineState::D3d12PipelineState(com_ptr<ID3D12PipelineState> state)
    : _state(std::move(state)) {
    GM_ASSERT(_state != nullptr);
}

gm::D3d12PipelineState::~D3d12PipelineState() = default;

auto gm::D3d12PipelineState::createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D12Device1* device) -> box<D3d12PipelineState> {
    GM_ASSERT(device != nullptr);

    D3D12_ROOT_SIGNATURE_DESC rootDesc = {};
    rootDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_INPUT_ELEMENT_DESC layout = {};
    layout.AlignedByteOffset = 0;
    layout.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    layout.InputSlot = 0;
    layout.SemanticName = "POSITION";

    com_ptr<ID3DBlob> rootBlob;
    com_ptr<ID3DBlob> rootErrorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, out_ptr(rootBlob), out_ptr(rootErrorBlob));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    com_ptr<ID3D12RootSignature> root;
    hr = device->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), out_ptr(root));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeDesc = {};
    pipeDesc.pRootSignature = root.get();
    pipeDesc.BlendState.AlphaToCoverageEnable = false;
    pipeDesc.BlendState.IndependentBlendEnable = false;
    pipeDesc.BlendState.RenderTarget[0].BlendEnable = true;
    pipeDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    pipeDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    pipeDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_COLOR;
    pipeDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    pipeDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_COLOR;
    pipeDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
    pipeDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pipeDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pipeDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    pipeDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeDesc.InputLayout.NumElements = 1;
    pipeDesc.InputLayout.pInputElementDescs = &layout;
    pipeDesc.NumRenderTargets = 1;
    pipeDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipeDesc.SampleDesc.Count = 1;
    pipeDesc.SampleDesc.Quality = 0;
    pipeDesc.VS.pShaderBytecode = desc.vertShader.data();
    pipeDesc.VS.BytecodeLength = desc.vertShader.size();

    com_ptr<ID3D12PipelineState> state;
    hr = device->CreateGraphicsPipelineState(&pipeDesc, __uuidof(ID3D12PipelineState), out_ptr(state));
    if (!SUCCEEDED(hr)) {
        return nullptr;
    }

    return make_box<D3d12PipelineState>(std::move(state));
}

auto gm::D3d12PipelineState::toNative(GpuPipelineState* state) -> ID3D12PipelineState* {
    auto d3d12State = static_cast<D3d12PipelineState*>(state);
    return d3d12State != nullptr ? d3d12State->_state.get() : nullptr;
}
