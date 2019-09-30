// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/stream.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/vector.h"
#include "potato/spud/string.h"

auto up::readBinary(Stream& stream, vector<up::byte>& out) -> IOResult {
    if (!stream.canRead() || !stream.canSeek()) {
        return IOResult::UnsupportedOperation;
    }

    auto const size = stream.remaining();
    auto const offset = out.size();
    out.resize(offset + size);

    auto read = out.subspan(offset);
    return stream.read(read);
}

auto up::readText(Stream& stream, string& out) -> IOResult {
    if (!stream.canRead() || !stream.canSeek()) {
        return IOResult::UnsupportedOperation;
    }

    auto const size = stream.remaining();
    string_writer writer;

    auto const uncommitted = writer.acquire(size);
    auto bytes = uncommitted.as_bytes();

    auto const rs = stream.read(bytes);

    writer.commit(uncommitted.first(bytes.size()));

    out = std::move(writer).to_string();

    return rs;
}

auto up::writeAllText(Stream& stream, string_view text) -> IOResult {
    if (!stream.canWrite()) {
        return IOResult::UnsupportedOperation;
    }

    auto const textSpan = span(text.data(), text.size());
    return stream.write(textSpan.as_bytes());
}
