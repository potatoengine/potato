// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/gpu/sampler.h"

namespace gm::gpu::d3d11 {
    class SamplerD3D11 final : public Sampler {
    public:
        explicit SamplerD3D11(com_ptr<ID3D11SamplerState> sampler);
        virtual ~SamplerD3D11();

        SamplerD3D11(SamplerD3D11&&) = delete;
        SamplerD3D11& operator=(SamplerD3D11&&) = delete;

        com_ptr<ID3D11SamplerState> const& get() const noexcept { return _sampler; }

    private:
        com_ptr<ID3D11SamplerState> _sampler;
    };
} // namespace gm::gpu::d3d11
