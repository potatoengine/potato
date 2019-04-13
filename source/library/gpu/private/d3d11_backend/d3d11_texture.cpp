// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#include "d3d11_texture.h"
#include "d3d11_platform.h"
#include "potato/gpu/com_ptr.h"
#include "potato/foundation/out_ptr.h"
#include "potato/foundation/assertion.h"

up::gpu::d3d11::TextureD3D11::TextureD3D11(com_ptr<ID3D11Resource> texture) : _texture(std::move(texture)) {
}

up::gpu::d3d11::TextureD3D11::~TextureD3D11() = default;

auto up::gpu::d3d11::TextureD3D11::type() const noexcept -> GpuTextureType {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        return GpuTextureType::Texture2D;
    }

    UP_UNREACHABLE("could not detect texture type");
    return GpuTextureType::Texture2D;
}

auto up::gpu::d3d11::TextureD3D11::format() const noexcept -> GpuFormat {
    return fromNative(nativeFormat());
}

DXGI_FORMAT up::gpu::d3d11::TextureD3D11::nativeFormat() const noexcept {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        return desc.Format;
    }
    return DXGI_FORMAT_UNKNOWN;
}

auto up::gpu::d3d11::TextureD3D11::dimensions() const noexcept -> glm::ivec3 {
    com_ptr<ID3D11Texture2D> texture2D;
    if (SUCCEEDED(_texture->QueryInterface(__uuidof(ID3D11Texture2D), out_ptr(texture2D)))) {
        D3D11_TEXTURE2D_DESC desc;
        texture2D->GetDesc(&desc);
        return {desc.Width, desc.Height, 0};
    }

    UP_UNREACHABLE("could not detect texture type");
    return {0, 0, 0};
}
