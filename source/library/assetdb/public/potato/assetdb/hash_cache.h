// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/span.h"
#include "potato/foundation/string.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/box.h"
#include "potato/foundation/int_types.h"
#include "potato/foundation/hash.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream.h"
#include <unordered_map>

namespace up {
    class HashCache {
    public:
        HashCache() = default;
        HashCache(fs::FileSystem fileSystem) : _fileSystem(std::move(fileSystem)) {}

        static UP_ASSETDB_API uint64 hashAssetContent(span<byte const> contents) noexcept;
        static UP_ASSETDB_API uint64 hashAssetStream(fs::Stream& stream);

        UP_ASSETDB_API uint64 hashAssetAtPath(zstring_view path);

        UP_ASSETDB_API bool serialize(fs::Stream& stream) const;
        UP_ASSETDB_API bool deserialize(fs::Stream& stream);

    private:
        struct HashRecord {
            string osPath;
            uint64 hash = 0;
            uint64 mtime = 0;
            uint64 size = 0;
        };

        fs::FileSystem _fileSystem;
        std::unordered_map<zstring_view, box<HashRecord>, uhash<>> _hashes;
    };
} // namespace up