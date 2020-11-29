// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_pipeline_state.h"
#include "d3d12_command_list.h"

#include "potato/runtime/assertion.h"
#include "potato/spud/out_ptr.h"

up::d3d12::PipelineStateD3D12::PipelineStateD3D12()  {
}

up::d3d12::PipelineStateD3D12::~PipelineStateD3D12() = default;

auto up::d3d12::PipelineStateD3D12::createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D12Device* device)
    -> box<PipelineStateD3D12> {
    UP_ASSERT(device != nullptr);

    return new_box<PipelineStateD3D12>();
}

void up::d3d12::PipelineStateD3D12::draw(CommandListD3D12& cmd) {

}
