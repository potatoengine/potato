// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/image/image.h"
#include "grimm/filesystem/stream.h"
#include <stb_image.h>

static int stb_read(void* user, char* data, int size) {
    auto* stream = static_cast<up::fs::Stream*>(user);

    auto bytes = up::span<char>{data, static_cast<size_t>(size)}.as_bytes();
    if (stream->read(bytes) != up::fs::Result::Success) {
        return 0;
    }

    return static_cast<int>(bytes.size());
}

static void stb_skip(void* user, int n) {
    auto* stream = static_cast<up::fs::Stream*>(user);

    stream->seek(up::fs::SeekPosition::Current, n);
}

static int stb_eof(void* user) {
    auto* stream = static_cast<up::fs::Stream*>(user);
    return stream->isEof();
}

static constexpr stbi_io_callbacks stb_io = {&stb_read, &stb_skip, &stb_eof};

auto up::loadImage(fs::Stream& stream) -> Image {
    ImageHeader header;

    int channels = 0;
    stbi_uc* image = stbi_load_from_callbacks(&stb_io, &stream, &header.width, &header.height, &channels, 4);
    if (image == nullptr) {
        return {};
    }

    vector<byte> data(header.width * header.width * 4);
    std::memcpy(data.data(), image, data.size());
    free(image);

    switch (channels) {
    case 1:
        header.pixelFormat = ImagePixelFormat::R8_UNORM;
        break;
    case 3:
        header.pixelFormat = ImagePixelFormat::RGB8_UNORM;
        break;
    case 4:
        header.pixelFormat = ImagePixelFormat::RGBA8_UNORM;
        break;
    }

    return Image(header, std::move(data));
}
