// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"

namespace gm::image {
    enum class ImageFormat {
        Unknown,
        RGBA8_UNORM,
        RGB8_UNORM,
        R8_UNORM,
        DXT5
    };

    GM_IMAGE_API bool isCompressed(ImageFormat format) noexcept;
    GM_IMAGE_API int channelCount(ImageFormat format) noexcept;
    GM_IMAGE_API int bytesPerPixel(ImageFormat format) noexcept;
} // namespace gm::image
