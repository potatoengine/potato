// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "d3d12_platform.h"
#include "gpu_buffer.h"

#include "potato/runtime/com_ptr.h"

namespace up::d3d12 {
    class BufferD3D12 final : public GpuBuffer {
    public:
        BufferD3D12(GpuBufferType type, uint64 size, ID3DResourcePtr buffer) noexcept;
        ~BufferD3D12();

        GpuBufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return _size; }

        ID3DResourcePtr const& buffer() const noexcept { return _buffer; }

    private:
        GpuBufferType _type;
        uint64 _size;
        ID3DResourcePtr _buffer;
    };
} // namespace up::d3d12
