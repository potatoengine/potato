// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "format.h"
#include "grimm/foundation/types.h"
#include "grimm/foundation/blob.h"

namespace gm::fs {
    class Stream;
}

namespace gm::image {
    class ImageView {
    public:
        ImageView() = default;
        ImageView(ImageFormat format, byte* data, int width, int height, int stride) noexcept : _data(data), _format(format), _width(width), _height(height), _stride(stride) {}

        byte* data() noexcept { return _data; }
        byte const* data() const noexcept { return _data; }

        std::size_t size() const noexcept { return _height * _stride; }

        ImageFormat format() const noexcept { return _format; }
        int width() const noexcept { return _width; }
        int height() const noexcept { return _height; }
        int stride() const noexcept { return _stride; }

        bool isCompact() const noexcept { return _width == _stride; }
        bool isCompressed() const noexcept { return gm::image::isCompressed(_format); }

    private:
        byte* _data = nullptr;
        ImageFormat _format = ImageFormat::Unknown;
        int _width = 0;
        int _height = 0;
        int _stride = 0;
    };

    class Image : public ImageView {
    public:
        Image() = default;
        Image(ImageFormat format, gm::blob data, int width, int height, int stride) noexcept : ImageView(format, data.data(), width, height, stride), _data(std::move(data)) {}

    private:
        gm::blob _data;
    };

    GM_IMAGE_API Image loadImage(fs::Stream& stream);
    GM_IMAGE_API bool saveDDS(ImageView const& image, fs::Stream& stream);
} // namespace gm::image
