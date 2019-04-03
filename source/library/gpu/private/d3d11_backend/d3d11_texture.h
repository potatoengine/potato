// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "d3d11_platform.h"
#include "grimm/gpu/com_ptr.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/texture.h"

namespace up::gpu::d3d11 {
    class TextureD3D11 final : public Texture {
    public:
        explicit TextureD3D11(com_ptr<ID3D11Resource> texture);
        virtual ~TextureD3D11();

        TextureD3D11(TextureD3D11&&) = delete;
        TextureD3D11& operator=(TextureD3D11&&) = delete;

        TextureType type() const noexcept override;
        Format format() const noexcept override;
        glm::ivec3 dimensions() const noexcept;

        DXGI_FORMAT nativeFormat() const noexcept;
        com_ptr<ID3D11Resource> const& get() const { return _texture; }

    private:
        com_ptr<ID3D11Resource> _texture;
    };
} // namespace up::gpu::d3d11
