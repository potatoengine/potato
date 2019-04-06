// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "d3d11_sampler.h"

up::gpu::d3d11::SamplerD3D11::SamplerD3D11(com_ptr<ID3D11SamplerState> sampler) : _sampler(std::move(sampler)) {}

up::gpu::d3d11::SamplerD3D11::~SamplerD3D11() = default;
