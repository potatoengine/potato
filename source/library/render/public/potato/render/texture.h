// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/image/image.h"

namespace up::gpu {
    class Texture;
}

namespace up {
    class RenderContext;

    class Texture : public shared<Texture> {
    public:
        UP_RENDER_API explicit Texture(Image image, box<gpu::Texture> texture);
        UP_RENDER_API ~Texture();

        gpu::Texture& texture() const noexcept { return *_texture; }

    private:
        box<gpu::Texture> _texture;
        Image _image;
    };
} // namespace up
