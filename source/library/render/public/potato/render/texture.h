// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/box.h"
#include "potato/foundation/rc.h"
#include "potato/image/image.h"

namespace up::gpu {
    class GpuTexture;
}

namespace up {
    class RenderContext;

    class GpuTexture : public shared<GpuTexture> {
    public:
        UP_RENDER_API explicit GpuTexture(Image image, box<gpu::GpuTexture> texture);
        UP_RENDER_API ~GpuTexture();

        gpu::GpuTexture& texture() const noexcept { return *_texture; }

    private:
        box<gpu::GpuTexture> _texture;
        Image _image;
    };
} // namespace up
