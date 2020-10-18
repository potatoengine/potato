// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "file_hash_cache.h"

#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"

#include <sqlite3.h>

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
    if (!_openCache())
        return false;

    unique_resource<sqlite3_stmt*, sqlite3_finalize> stmt;
    {
        auto const rs = sqlite3_prepare_v3(
            _cacheDb.get(),
            "INSERT INTO hash_cache (os_path, hash, mtime, size) VALUES(?, ?, ?, ?)",
            -1,
            0,
            out_ptr(stmt),
            nullptr);
    }

    for (auto const& [key, value] : _hashes) {
        sqlite3_reset(stmt.get());
        sqlite3_bind_text(stmt.get(), 1, key.data(), key.size(), nullptr);
        sqlite3_bind_int64(stmt.get(), 2, value->hash);
        sqlite3_bind_int64(stmt.get(), 3, value->mtime);
        sqlite3_bind_int64(stmt.get(), 4, value->size);
        sqlite3_step(stmt.get());
    }

    return true;
}

bool up::FileHashCache::loadCache(zstring_view cache_path) {
    _cachePath = cache_path;
    _cacheDb.reset();

    if (!_openCache())
        return false;

    unique_resource<sqlite3_stmt*, sqlite3_finalize> stmt;
    {
        auto const rs = sqlite3_prepare_v3(
            _cacheDb.get(),
            "SELECT os_path, hash, mtime, size FROM hash_cache",
            -1,
            0,
            out_ptr(stmt),
            nullptr);
    }

    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        auto rec = new_box<HashRecord>();
        rec->osPath = reinterpret_cast<char const*>(sqlite3_column_text(stmt.get(), 1));
        rec->hash = sqlite3_column_int64(stmt.get(), 2);
        rec->mtime = sqlite3_column_int64(stmt.get(), 3);
        rec->size = sqlite3_column_int64(stmt.get(), 4);
    }

    return true;
}

bool up::FileHashCache::_openCache() {
    if (_cacheDb != nullptr)
        return true;

    // open database
    {
        sqlite3* db = nullptr;
        auto const rs = sqlite3_open(_cachePath.c_str(), &db);
        if (rs != SQLITE_OK)
            return false;

        _cacheDb.reset(db);
    }

    // ensure cache table exists
    {
        auto const rs = sqlite3_exec(
            _cacheDb.get(),
            "CREATE TABLE IF NOT EXISTS hash_cache (os_path STRING, hash INTEGER, mtime INTEGER, size INTEGER)",
            nullptr,
            nullptr,
            nullptr);
        if (rs != SQLITE_OK) {
            _cacheDb.reset();
            return false;
        }
    }

    return true;
}
