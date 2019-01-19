// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/library/asset_hashes.h"
#include "grimm/foundation/fnv1a.h"

auto gm::AssetHashes::hashAssetContent(span<gm::byte const> contents) noexcept -> gm::uint64 {
    auto hasher = fnv1a();
    hasher(contents);
    return static_cast<uint64>(hasher);
}

auto gm::AssetHashes::hashAssetStream(std::istream& stream) -> gm::uint64 {
    auto hasher = fnv1a();
    char buffer[32768];
    while (stream) {
        stream.read(buffer, sizeof(buffer));
        auto count = stream.gcount();
        hasher(span(reinterpret_cast<byte const*>(buffer), count));
    }
    return static_cast<uint64>(hasher);
}

auto gm::AssetHashes::hashAssetAtPath(zstring_view path) -> gm::uint64 {
    auto fstream = _fileSystem.openRead(path);
    return hashAssetStream(fstream);
}
