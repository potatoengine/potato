// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "file_hash_cache.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"

up::FileHashCache::FileHashCache() = default;

up::FileHashCache::~FileHashCache() = default;

auto up::FileHashCache::hashAssetContent(span<up::byte const> contents) noexcept -> up::uint64 {
    return hash_value<fnv1a>(contents);
}

auto up::FileHashCache::hashAssetStream(Stream& stream) -> up::uint64 {
    constexpr int block_size = 8 * 1024;

    auto hasher = fnv1a();
    up::byte buffer[block_size];
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

auto up::FileHashCache::hashAssetAtPath(zstring_view path) -> up::uint64 {
    auto const [rs, stat] = fs::fileStat(path);
    if (rs != IOResult::Success) {
        return 0;
    }

    auto it = _hashes.find(path);
    if (it != _hashes.end()) {
        if (rs == IOResult::Success && stat.size == it->second->size && stat.mtime == it->second->mtime) {
            return it->second->hash;
        }
    }

    auto fstream = fs::openRead(path);
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

bool up::FileHashCache::saveCache() {
    if (!_conn)
        return false;

    auto stmt = _conn.prepare("INSERT INTO hash_cache (os_path, hash, mtime, size) VALUES(?, ?, ?, ?)");

    for (auto const& [key, value] : _hashes) {
        (void)stmt.execute(key.c_str(), value->hash, value->mtime, value->size);
    }

    return true;
}

bool up::FileHashCache::loadCache(zstring_view cache_path) {
    _conn.close();

    auto const rs = _conn.open(cache_path);
    if (rs != SqlResult::Ok)
        return false;

    // ensure cache table exists
    if (_conn.execute(
            "CREATE TABLE IF NOT EXISTS hash_cache (os_path STRING, hash INTEGER, mtime INTEGER, size INTEGER)") !=
        SqlResult::Ok) {
        _conn.close();
        return false;
    }

    // load cache entries
    auto stmt = _conn.prepare("SELECT os_path, hash, mtime, size FROM hash_cache");

    for (auto const& row : stmt.query<zstring_view, uint64, uint64, uint64>()) {
        auto rec_ptr = new_box<HashRecord>();
        auto& rec = *rec_ptr;
        std::tie(rec.osPath, rec.hash, rec.mtime, rec.size) = row;
    }

    return true;
}
