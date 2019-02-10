// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "format.h"
#include "grimm/foundation/assertion.h"

bool gm::image::isCompressed(ImageFormat format) noexcept {
    switch (format) {
    case ImageFormat::DXT5:
        return true;
    default:
        return false;
    }
}

int gm::image::channelCount(ImageFormat format) noexcept {
    switch (format) {
    case ImageFormat::RGBA8_UNORM:
    case ImageFormat::DXT5:
        return 4;
    case ImageFormat::RGB8_UNORM:
        return 3;
    case ImageFormat::R8_UNORM:
        return 1;
    default:
        GM_UNREACHABLE("Unknown format");
        return 0;
    }
}

int gm::image::bytesPerPixel(ImageFormat format) noexcept {
    switch (format) {
    case ImageFormat::RGBA8_UNORM:
        return 4;
    case ImageFormat::RGB8_UNORM:
        return 3;
    case ImageFormat::R8_UNORM:
    case ImageFormat::DXT5:
        return 1;
    default:
        GM_UNREACHABLE("Unknown format");
        return 0;
    }
}
