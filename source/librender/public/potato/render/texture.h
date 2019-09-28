// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/image/image.h"

namespace up {
    class GpuTexture;
}

namespace up {
    class Texture : public shared<Texture> {
    public:
        UP_RENDER_API explicit Texture(Image image, box<GpuTexture> texture);
        UP_RENDER_API ~Texture();

        GpuTexture& texture() const noexcept { return *_texture; }

    private:
        box<GpuTexture> _texture;
        Image _image;
    };
} // namespace up
