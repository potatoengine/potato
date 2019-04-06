// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/gpu/buffer.h"

namespace up::gpu::d3d11 {
    class BufferD3D11 final : public Buffer {
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
} // namespace up::gpu::d3d11
