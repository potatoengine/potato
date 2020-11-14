// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "d3d12_buffer.h"

#include "potato/spud/int_types.h"

up::d3d12::BufferD3D12::BufferD3D12(GpuBufferType type, up::uint64 size, ID3DResourcePtr buffer) noexcept
    : _type(type)
    , _size(size)
    , _buffer(std::move(buffer)) {}

up::d3d12::BufferD3D12::~BufferD3D12() = default;
