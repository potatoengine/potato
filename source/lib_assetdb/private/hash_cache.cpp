// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/assetdb/hash_cache.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/json.h"
#include <nlohmann/json.hpp>

auto up::HashCache::hashAssetContent(span<up::byte const> contents) noexcept -> up::uint64 {
    return hash_value<fnv1a>(contents);
}

auto up::HashCache::hashAssetStream(Stream& stream) -> up::uint64 {
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
    FileStat stat;
    auto rs = _fileSystem.fileStat(path, stat);
    if (rs != IOResult::Success) {
        return 0;
    }

    auto it = _hashes.find(path);
    if (it != _hashes.end()) {
        if (rs == IOResult::Success && stat.size == it->second->size && stat.mtime == it->second->mtime) {
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

bool up::HashCache::serialize(Stream& stream) const {
    nlohmann::json jsonRoot;

    for (auto const& [key, value] : _hashes) {
        nlohmann::json jsonRecord;

        jsonRecord["name"] = std::string(key.data(), key.size());
        jsonRecord["hash"] = value->hash;
        jsonRecord["mtime"] = value->mtime;
        jsonRecord["size"] = value->size;

        jsonRoot.push_back(std::move(jsonRecord));
    }

    auto json = jsonRoot.dump();
    return writeAllText(stream, {json.data(), json.size()}) == IOResult::Success;
}

bool up::HashCache::deserialize(Stream& stream) {
    string jsonText;
    if (readText(stream, jsonText) != IOResult::Success) {
        return false;
    }

    auto jsonRoot = nlohmann::json::parse(jsonText, nullptr, false);

    if (jsonRoot.is_discarded()) {
        return false;
    }

    for (auto& member : jsonRoot) {
        uint64 hash = member["hash"];        
        uint64 mtime = member["mtime"];
        uint64 size = member["size"];

        auto path = member["name"].get<string>();

        auto rec = new_box<HashRecord>();
        rec->osPath = std::move(path);
        rec->hash = hash;
        rec->mtime = mtime;
        rec->size = size;
        _hashes[rec->osPath] = std::move(rec);
        return hash;
    }

    return true;
}
