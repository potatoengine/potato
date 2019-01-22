// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_texture.h"
#include "d3d11_platform.h"
#include "com_ptr.h"
#include "grimm/foundation/out_ptr.h"
#include "grimm/foundation/assertion.h"

gm::gpu::d3d11::TextureD3D11::TextureD3D11(com_ptr<ID3D11Resource> texture) : _texture(std::move(texture)) {
}

gm::gpu::d3d11::TextureD3D11::~TextureD3D11() = default;

auto gm::gpu::d3d11::TextureD3D11::type() const noexcept -> TextureType {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        return TextureType::Texture2D;
    }

    GM_UNREACHABLE("could not detect texture type");
    return TextureType::Texture2D;
}

auto gm::gpu::d3d11::TextureD3D11::dimensions() const noexcept -> PackedVector3f {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        return {static_cast<float>(desc.Width), static_cast<float>(desc.Height), 0};
    }

    GM_UNREACHABLE("could not detect texture type");
    return {0, 0, 0};
}
