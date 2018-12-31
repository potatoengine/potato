// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#if GM_GPU_ENABLE_D3D12

#    include "d3d12_pipeline_state.h"
#    include "grimm/foundation/assert.h"
#    include "grimm/foundation/out_ptr.h"

gm::D3d12PipelineState::D3d12PipelineState(com_ptr<ID3D12PipelineState> state)
    : _state(std::move(state)) {
    GM_ASSERT(_state != nullptr);
}

gm::D3d12PipelineState::~D3d12PipelineState() = default;

auto gm::D3d12PipelineState::createGraphicsPipelineState(ID3D12Device1* device) -> box<D3d12PipelineState> {
    GM_ASSERT(device != nullptr);

    D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};

    com_ptr<ID3D12PipelineState> state;
    HRESULT hr = device->CreateGraphicsPipelineState(&desc, __uuidof(ID3D12PipelineState), out_ptr(state));
    if (state == nullptr) {
        return nullptr;
    }

    return make_box<D3d12PipelineState>(std::move(state));
}

auto gm::D3d12PipelineState::toNative(GpuPipelineState* state) -> ID3D12PipelineState* {
    auto d3d12State = static_cast<D3d12PipelineState*>(state);
    return d3d12State != nullptr ? d3d12State->_state.get() : nullptr;
}

#endif
