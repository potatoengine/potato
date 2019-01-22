// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/gpu/buffer.h"

namespace gm {
    class BufferD3D11 final : public GpuBuffer {
    public:
        BufferD3D11(BufferType type, uint64 size, com_ptr<ID3D11Buffer> buffer);
        ~BufferD3D11();

        BufferType type() const noexcept override { return _type; }
        uint64 size() const noexcept override { return _size; }

        com_ptr<ID3D11Buffer> const& buffer() const noexcept { return _buffer; }

    private:
        BufferType _type;
        uint64 _size;
        com_ptr<ID3D11Buffer> _buffer;
    };
} // namespace gm
