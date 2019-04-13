// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/gpu/sampler.h"

namespace up::d3d11 {
    class SamplerD3D11 final : public GpuSampler {
    public:
        explicit SamplerD3D11(com_ptr<ID3D11SamplerState> sampler);
        virtual ~SamplerD3D11();

        SamplerD3D11(SamplerD3D11&&) = delete;
        SamplerD3D11& operator=(SamplerD3D11&&) = delete;

        com_ptr<ID3D11SamplerState> const& get() const noexcept { return _sampler; }

    private:
        com_ptr<ID3D11SamplerState> _sampler;
    };
} // namespace up::d3d11
