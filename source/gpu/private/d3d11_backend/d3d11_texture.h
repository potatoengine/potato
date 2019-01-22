// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "com_ptr.h"
#include "d3d11_platform.h"
#include "grimm/foundation/box.h"
#include "grimm/gpu/texture.h"

namespace gm::gpu::d3d11 {
    class TextureD3D11 final : public Texture {
    public:
        explicit TextureD3D11(com_ptr<ID3D11Resource> texture);
        virtual ~TextureD3D11();

        TextureD3D11(TextureD3D11&&) = delete;
        TextureD3D11& operator=(TextureD3D11&&) = delete;

        com_ptr<ID3D11Resource> const& get() const { return _texture; }

    private:
        com_ptr<ID3D11Resource> _texture;
    };
} // namespace gm::gpu::d3d11
