// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "direct3d.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/pipeline_state.h"

namespace gm {
    class D3d12PipelineState : public GpuPipelineState {
    public:
        explicit D3d12PipelineState(com_ptr<ID3D12PipelineState> state);
        virtual ~D3d12PipelineState();

        static box<D3d12PipelineState> createGraphicsPipelineState(GpuPipelineStateDesc const& desc, ID3D12Device1* device);

        static ID3D12PipelineState* toNative(GpuPipelineState* state);

    private:
        com_ptr<ID3D12PipelineState> _state;
    };
} // namespace gm
