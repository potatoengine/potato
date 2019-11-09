// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/box.h"
#include "potato/spud/int_types.h"
#include "potato/spud/hash.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/stream.h"
#include <unordered_map>

namespace up {
    class HashCache {
    public:
        explicit HashCache(FileSystem& fileSystem) : _fileSystem(fileSystem) {}

        static UP_ASSETDB_API uint64 hashAssetContent(span<byte const> contents) noexcept;
        static UP_ASSETDB_API uint64 hashAssetStream(Stream& stream);

        UP_ASSETDB_API uint64 hashAssetAtPath(zstring_view path);

        UP_ASSETDB_API bool serialize(Stream& stream) const;
        UP_ASSETDB_API bool deserialize(Stream& stream);

    private:
        struct HashRecord {
            string osPath;
            uint64 hash = 0;
            uint64 mtime = 0;
            uint64 size = 0;
        };

        FileSystem& _fileSystem;
        std::unordered_map<zstring_view, box<HashRecord>, uhash<>> _hashes;
    };
} // namespace up
