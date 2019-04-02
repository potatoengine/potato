// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/string.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/box.h"
#include "grimm/foundation/int_types.h"
#include "grimm/foundation/hash.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream.h"
#include <unordered_map>

namespace gm {
    class HashCache {
    public:
        HashCache() = default;
        HashCache(fs::FileSystem fileSystem) : _fileSystem(std::move(fileSystem)) {}

        static GM_ASSETDB_API uint64 hashAssetContent(span<byte const> contents) noexcept;
        static GM_ASSETDB_API uint64 hashAssetStream(fs::Stream& stream);

        GM_ASSETDB_API uint64 hashAssetAtPath(zstring_view path);

        GM_ASSETDB_API bool serialize(fs::Stream& stream) const;
        GM_ASSETDB_API bool deserialize(fs::Stream& stream);

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
} // namespace gm
