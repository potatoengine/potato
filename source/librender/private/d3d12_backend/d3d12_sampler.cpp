// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_sampler.h"

up::d3d12::SamplerD3D12::SamplerD3D12(com_ptr<ID3D12SamplerState> sampler) : _sampler(std::move(sampler)) {}

up::d3d12::SamplerD3D12::~SamplerD3D12() = default;
