// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_sampler.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d12 {
    class SamplerD3D12 final : public GpuSampler {
    public:
        explicit SamplerD3D12(com_ptr<ID3D12SamplerState> sampler);
        virtual ~SamplerD3D12();

        SamplerD3D12(SamplerD3D12&&) = delete;
        SamplerD3D12& operator=(SamplerD3D12&&) = delete;

        com_ptr<ID3D12SamplerState> const& get() const noexcept { return _sampler; }

    private:
        com_ptr<ID3D12SamplerState> _sampler;
    };
} // namespace up::d3d12
