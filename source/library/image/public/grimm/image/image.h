// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "pixel_format.h"
#include "grimm/foundation/int_types.h"
#include "grimm/foundation/vector.h"

namespace up::fs {
    class Stream;
}

namespace up {
    struct ImageHeader {
        ImagePixelFormat pixelFormat = ImagePixelFormat::Unknown;
        int width = 0, height = 0, stride = 0;
    };

    class Image {
    public:
        Image() = default;
        Image(ImageHeader header, vector<byte> data) noexcept : _data(std::move(data)), _header(header) {}

        ImageHeader const& header() const noexcept { return _header; }
        vector<byte> const& data() const noexcept { return _data; }

        bool isCompact() const noexcept { return _header.width == _header.stride; }
        bool isCompressed() const noexcept { return up::isCompressed(_header.pixelFormat); }

    private:
        vector<byte> _data;
        ImageHeader _header;
    };

    UP_IMAGE_API Image loadImage(fs::Stream& stream);
    UP_IMAGE_API bool saveDDS(Image const& image, fs::Stream& stream);
} // namespace up
