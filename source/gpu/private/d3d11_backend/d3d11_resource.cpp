// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_resource.h"
#include "com_ptr.h"
#include "d3d11_platform.h"

gm::gpu::d3d11::ResourceD3D11::ResourceD3D11(com_ptr<ID3D11Resource> resource) : _resource(std::move(resource)) {}

gm::gpu::d3d11::ResourceD3D11::~ResourceD3D11() = default;
