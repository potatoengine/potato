// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_resource_view.h"

up::d3d12::ResourceViewD3D12::ResourceViewD3D12(GpuViewType type, com_ptr<ID3D12View> view)
    : _type(type)
    , _view(std::move(view)) {}

up::d3d12::ResourceViewD3D12::~ResourceViewD3D12() = default;
