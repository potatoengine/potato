// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/pipeline_state.h"

namespace up::gpu::d3d11 {
    struct PipelineStateParamsD3D11 {
        com_ptr<ID3D11RasterizerState> rasterState;
        com_ptr<ID3D11DepthStencilState> depthStencilState;
        com_ptr<ID3D11BlendState> blendState;
        com_ptr<ID3D11InputLayout> inputLayout;
        com_ptr<ID3D11VertexShader> vertShader;
        com_ptr<ID3D11PixelShader> pixelShader;
    };

    class PipelineStateD3D11 : public PipelineState {
    public:
        explicit PipelineStateD3D11(PipelineStateParamsD3D11 params);
        virtual ~PipelineStateD3D11();

        static box<PipelineStateD3D11> createGraphicsPipelineState(PipelineStateDesc const& desc, ID3D11Device* device);

        PipelineStateParamsD3D11 const& params() const noexcept { return _params; }

    private:
        static constexpr uint32 maxInputLayoutElements = 32;

        PipelineStateParamsD3D11 _params;
    };
} // namespace up::gpu::d3d11
