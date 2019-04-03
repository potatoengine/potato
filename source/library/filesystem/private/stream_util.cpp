// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/stream_util.h"
#include "potato/filesystem/stream.h"
#include "potato/foundation/string_writer.h"

auto up::fs::readBinary(Stream& stream, vector<up::byte>& out) -> Result {
    if (!stream.canRead() || !stream.canSeek()) {
        return Result::UnsupportedOperation;
    }

    auto size = stream.remaining();
    auto offset = out.size();
    out.resize(offset + size);

    auto read = out.subspan(offset);
    return stream.read(read);
}

auto up::fs::readText(Stream& stream, string& out) -> Result {
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
