// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/assetdb/hash_cache.h"
#include "potato/foundation/hash_fnv1a.h"
#include "potato/filesystem/stream_util.h"
#include "potato/filesystem/json_util.h"
#include <nlohmann/json.hpp>

auto up::HashCache::hashAssetContent(span<up::byte const> contents) noexcept -> up::uint64 {
    return hash_value<fnv1a>(contents);
}

auto up::HashCache::hashAssetStream(fs::Stream& stream) -> up::uint64 {
    auto hasher = fnv1a();
    up::byte buffer[32768];
    while (!stream.isEof()) {
        span<up::byte> read(buffer, sizeof(buffer));
        stream.read(read);
        if (read.empty()) {
            break;
        }
        hash_append(hasher, read);
    }
    return static_cast<uint64>(hasher.finalize());
}

auto up::HashCache::hashAssetAtPath(zstring_view path) -> up::uint64 {
    fs::FileStat stat;
    auto rs = _fileSystem.fileStat(path, stat);
    if (rs != fs::Result::Success) {
        return 0;
    }

    auto it = _hashes.find(path);
    if (it != _hashes.end()) {
        if (rs == fs::Result::Success && stat.size == it->second->size && stat.mtime == it->second->mtime) {
            return it->second->hash;
        }
    }

    auto fstream = _fileSystem.openRead(path);
    auto hash = hashAssetStream(fstream);

    // update the hash
    auto rec = new_box<HashRecord>();
    rec->osPath = string(path);
    rec->hash = hash;
    rec->mtime = stat.mtime;
    rec->size = stat.size;
    _hashes[rec->osPath] = std::move(rec);
    return hash;
}

bool up::HashCache::serialize(fs::Stream& stream) const {
    nlohmann::json jsonRoot;

    for (auto const& [key, value] : _hashes) {
        nlohmann::json jsonRecord;
        
        jsonRecord["hash"] = value->hash;
        jsonRecord["mtime"] = value->mtime;
        jsonRecord["size"] = value->size;

        jsonRoot.push_back(std::move(jsonRecord));
    }

    auto json = jsonRoot.dump();
    return writeAllText(stream, {json.data(), json.size()}) == fs::Result::Success;
}

bool up::HashCache::deserialize(fs::Stream& stream) {
    string jsonText;
    if (readText(stream, jsonText) != fs::Result::Success) {
        return false;
    }

    auto jsonRoot = nlohmann::json::parse(jsonText, nullptr, false);

    if (!jsonRoot.is_array()) {
        return false;
    }

    for (auto const& member : jsonRoot) {
        uint64 hash = member["hash"];        
        uint64 mtime = member["mtime"];
        uint64 size = member["size"];

        string path(member["name"].get<string>());

        auto rec = new_box<HashRecord>();
        rec->osPath = string(path);
        rec->hash = hash;
        rec->mtime = mtime;
        rec->size = size;
        _hashes[rec->osPath] = std::move(rec);
        return hash;
    }

    return true;
}
