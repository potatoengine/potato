// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "pixel_format.h"
#include "grimm/foundation/assertion.h"

bool gm::isCompressed(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::DXT5:
        return true;
    default:
        return false;
    }
}

int gm::channelCount(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::RGBA8_UNORM:
    case ImagePixelFormat::DXT5:
        return 4;
    case ImagePixelFormat::RGB8_UNORM:
        return 3;
    case ImagePixelFormat::R8_UNORM:
        return 1;
    default:
        GM_UNREACHABLE("Unknown format");
        return 0;
    }
}

int gm::bytesPerPixel(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::RGBA8_UNORM:
        return 4;
    case ImagePixelFormat::RGB8_UNORM:
        return 3;
    case ImagePixelFormat::R8_UNORM:
    case ImagePixelFormat::DXT5:
        return 1;
    default:
        GM_UNREACHABLE("Unknown format");
        return 0;
    }
}
