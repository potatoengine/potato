// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/runtime/stream.h"
#include "potato/spud/box.h"
#include "potato/spud/hash.h"
#include "potato/spud/int_types.h"
#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/zstring_view.h"

#include <unordered_map>

extern "C" {
struct sqlite3;
int sqlite3_close_v2(sqlite3*);
}

namespace up {
    class FileHashCache {
    public:
        UP_TOOLS_API FileHashCache();
        UP_TOOLS_API ~FileHashCache();

        FileHashCache(FileHashCache const&) = delete;
        FileHashCache& operator=(FileHashCache const&) = delete;

        static UP_TOOLS_API uint64 hashAssetContent(span<byte const> contents) noexcept;
        static UP_TOOLS_API uint64 hashAssetStream(Stream& stream);

        UP_TOOLS_API uint64 hashAssetAtPath(zstring_view path);

        UP_TOOLS_API bool loadCache(zstring_view cache_path);
        UP_TOOLS_API bool saveCache();

    private:
        struct HashRecord {
            string osPath;
            uint64 hash = 0;
            uint64 mtime = 0;
            uint64 size = 0;
        };

        std::unordered_map<zstring_view, box<HashRecord>, uhash<>> _hashes;
        unique_resource<sqlite3*, sqlite3_close_v2> _cacheDb;
        string _cachePath;

        bool _openCache();
    };
} // namespace up
