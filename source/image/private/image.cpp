// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "image.h"
#include "grimm/filesystem/stream.h"
#include <stb_image.h>

static int stb_read(void* user, char* data, int size) {
    auto* stream = static_cast<gm::fs::Stream*>(user);

    auto bytes = gm::span{data, static_cast<size_t>(size)}.as_bytes();
    if (stream->read(bytes) != gm::fs::Result::Success) {
        return 0;
    }

    return static_cast<int>(bytes.size_bytes());
}

static void stb_skip(void* user, int n) {
    auto* stream = static_cast<gm::fs::Stream*>(user);

    stream->seek(gm::fs::SeekPosition::Current, n);
}

static int stb_eof(void* user) {
    auto* stream = static_cast<gm::fs::Stream*>(user);
    return stream->isEof();
}

static constexpr stbi_io_callbacks stb_io = {&stb_read, &stb_skip, &stb_eof};

auto gm::image::loadImage(fs::Stream& stream) -> Image {
    int width = 0, height = 0, channels = 0;
    stbi_uc* image = stbi_load_from_callbacks(&stb_io, &stream, &width, &height, &channels, 0);
    if (image == nullptr) {
        return {};
    }

    gm::blob data(width * height * channels);
    std::memcpy(data.data(), image, data.size());
    free(image);

    ImageFormat format = ImageFormat::Unknown;
    switch (channels) {
    case 1:
        format = ImageFormat::R8_UNORM;
        break;
    case 3:
        format = ImageFormat::RGB8_UNORM;
        break;
    case 4:
        format = ImageFormat::RGBA8_UNORM;
        break;
    }

    return Image(format, std::move(data), width, height, channels * width);
}
