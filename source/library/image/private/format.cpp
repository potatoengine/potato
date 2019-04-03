// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/image/pixel_format.h"
#include "grimm/foundation/assertion.h"

bool up::isCompressed(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::DXT5:
        return true;
    default:
        return false;
    }
}

int up::channelCount(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::RGBA8_UNORM:
    case ImagePixelFormat::DXT5:
        return 4;
    case ImagePixelFormat::RGB8_UNORM:
        return 3;
    case ImagePixelFormat::R8_UNORM:
        return 1;
    default:
        UP_UNREACHABLE("Unknown format");
        return 0;
    }
}

int up::bytesPerPixel(ImagePixelFormat format) noexcept {
    switch (format) {
    case ImagePixelFormat::RGBA8_UNORM:
        return 4;
    case ImagePixelFormat::RGB8_UNORM:
        return 3;
    case ImagePixelFormat::R8_UNORM:
    case ImagePixelFormat::DXT5:
        return 1;
    default:
        UP_UNREACHABLE("Unknown format");
        return 0;
    }
}
