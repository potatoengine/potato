// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d12_resource.h"
#include "com_ptr.h"
#include "direct3d.h"

gm::D3d12Resource::D3d12Resource(com_ptr<ID3D12Resource> resource) : _resource(std::move(resource)) {}

gm::D3d12Resource::~D3d12Resource() = default;
