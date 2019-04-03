// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "d3d11_resource_view.h"

up::gpu::d3d11::ResourceViewD3D11::ResourceViewD3D11(ViewType type, com_ptr<ID3D11View> view)
    : _type(type),
      _view(std::move(view)) {
}

up::gpu::d3d11::ResourceViewD3D11::~ResourceViewD3D11() = default;
