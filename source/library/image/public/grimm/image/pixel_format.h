// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace gm {
    enum class ImagePixelFormat {
        Unknown,
        RGBA8_UNORM,
        RGB8_UNORM,
        R8_UNORM,
        DXT5
    };

    GM_IMAGE_API bool isCompressed(ImagePixelFormat format) noexcept;
    GM_IMAGE_API int channelCount(ImagePixelFormat format) noexcept;
    GM_IMAGE_API int bytesPerPixel(ImagePixelFormat format) noexcept;
} // namespace gm
