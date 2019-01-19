// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/span.h"
#include "grimm/foundation/types.h"
#include "grimm/filesystem/filesystem.h"
#include <string>
#include <fstream>
#include <unordered_map>

namespace gm {
    struct HashRecord {
        std::string osPath;
        uint64 hash = 0;
        uint64 mtime = 0;
        uint64 size = 0;
    };

    class AssetHashes {
    public:
        static GM_LIBRARY_API uint64 hashAssetContent(span<byte const> contents) noexcept;
        static GM_LIBRARY_API uint64 hashAssetStream(std::ifstream& stream);

        GM_LIBRARY_API uint64 hashAssetAtPath(zstring_view path);

    private:
        fs::FileSystem _fileSystem;
        std::unordered_map<std::string, HashRecord> _hashes;
    };
} // namespace gm
