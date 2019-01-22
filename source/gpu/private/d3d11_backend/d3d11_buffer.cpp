// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "d3d11_buffer.h"
#include "grimm/foundation/types.h"

gm::gpu::d3d11::BufferD3D11::BufferD3D11(BufferType type, gm::uint64 size, com_ptr<ID3D11Buffer> buffer)
    : _type(type),
      _size(size),
      _buffer(std::move(buffer)) {
}

gm::gpu::d3d11::BufferD3D11::~BufferD3D11() = default;
