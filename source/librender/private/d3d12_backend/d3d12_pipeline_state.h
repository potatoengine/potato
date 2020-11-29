// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_pipeline_state.h"

#include "potato/runtime/com_ptr.h"
#include "potato/spud/box.h"

namespace up::d3d12 {

    class CommandListD3D12;

    class PipelineStateD3D12 : public GpuPipelineState {
    public:
        explicit PipelineStateD3D12();
        virtual ~PipelineStateD3D12();

        static box<PipelineStateD3D12> createGraphicsPipelineState(
            GpuPipelineStateDesc const& desc,
            ID3D12Device* device);

        void draw(CommandListD3D12& cmd);

        const ID3DPipelineStatePtr& getState() const { return _state; }

    private:

        ID3DPipelineStatePtr _state;
    };
} // namespace up::d3d12
