// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_texture.h"
#include "com_ptr.h"
#include "d3d11_platform.h"

gm::gpu::d3d11::TextureD3D11::TextureD3D11(com_ptr<ID3D11Resource> texture) : _texture(std::move(texture)) {}

gm::gpu::d3d11::TextureD3D11::~TextureD3D11() = default;
