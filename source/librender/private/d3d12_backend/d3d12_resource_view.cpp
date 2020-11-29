// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_resource_view.h"

up::d3d12::ResourceViewD3D12::ResourceViewD3D12(GpuViewType type)
    : _type(type)
{}

up::d3d12::ResourceViewD3D12::~ResourceViewD3D12() = default;
