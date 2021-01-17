// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_resource_view.h"

up::d3d12::ResourceViewD3D12::ResourceViewD3D12(GpuViewType type)
    : _type(type)
{}

up::d3d12::ResourceViewD3D12::~ResourceViewD3D12() = default;

void up::d3d12::ResourceViewD3D12::create(DescriptorHeapD3D12* heap, D3D12_GPU_DESCRIPTOR_HANDLE hGpu, D3D12_CPU_DESCRIPTOR_HANDLE hCpu) {
    _heap = heap;
    _hGpu = hGpu;
    _hCpu = hCpu;
}
