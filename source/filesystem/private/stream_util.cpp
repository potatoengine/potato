// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/stream_util.h"
#include "grimm/filesystem/stream.h"
#include "grimm/foundation/string_writer.h"

auto gm::fs::readBlob(Stream& stream, blob& out) -> Result {
    if (!stream.canRead() || !stream.canSeek()) {
        return Result::UnsupportedOperation;
    }

    auto size = stream.remaining();
    out = blob(size);

    auto read = span<byte>{out.data_bytes(), out.size()};
    return stream.read(read);
}

auto gm::fs::readText(Stream& stream, string& out) -> Result {
    if (!stream.canRead() || !stream.canSeek()) {
        return Result::UnsupportedOperation;
    }

    auto size = stream.remaining();
    string_writer writer;

    auto uncommitted = writer.acquire(size);
    auto bytes = uncommitted.as_bytes();

    auto rs = stream.read(bytes);

    writer.commit(uncommitted.first(bytes.size()));

    out = std::move(writer).to_string();

    return rs;
}
