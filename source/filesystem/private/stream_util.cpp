// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/filesystem/stream_util.h"
#include "grimm/filesystem/stream.h"
#include "grimm/foundation/string_writer.h"

auto gm::fs::readBinary(Stream& stream, vector<gm::byte>& out) -> Result {
    if (!stream.canRead() || !stream.canSeek()) {
        return Result::UnsupportedOperation;
    }

    auto size = stream.remaining();
    out.resize(out.size() + size);

    auto read = out.subspan(out.size());
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
