// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_sampler.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d12 {
    class SamplerD3D12 final : public GpuSampler {
    public:
        explicit SamplerD3D12();
        virtual ~SamplerD3D12();

        SamplerD3D12(SamplerD3D12&&) = delete;
        SamplerD3D12& operator=(SamplerD3D12&&) = delete;

  
    private:
    };
} // namespace up::d3d12
