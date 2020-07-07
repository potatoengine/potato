// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_loader.h"
#include "filesystem.h"
#include "path.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

up::ResourceLoader::ResourceLoader() = default;

up::ResourceLoader::~ResourceLoader() = default;

auto up::ResourceLoader::assetPath(ResourceId id) const -> string {
    for (auto const& record : _manifest.records()) {
        if (record.logicalId == id) {
            return casPath(record.hash);
        }
    }
    return {};
}

auto up::ResourceLoader::assetPath(ResourceId id, string_view logicalName) const -> string {
    auto const hash = hash_value(logicalName);
    for (auto const& record : _manifest.records()) {
        if (record.rootId == id && record.logicalName == hash) {
            return casPath(record.hash);
        }
    }
    return {};
}

auto up::ResourceLoader::assetHash(string_view assetName) const -> ResourceId { return ResourceId{hash_value<fnv1a>(assetName)}; }

auto up::ResourceLoader::assetHash(string_view assetName, string_view logicalName) const -> ResourceId {
    fnv1a engine;
    hash_append(engine, assetName);
    if (!logicalName.empty()) {
        hash_append(engine, ":"_zsv);
        hash_append(engine, logicalName);
    }
    return ResourceId{engine.finalize()};
}

auto up::ResourceLoader::casPath(uint64 contentHash) const -> string {
    fixed_string_writer<32> casFilePath;
    format_append(casFilePath, "{:02X}/{:04X}/{:016X}.bin", (contentHash >> 56) & 0xFF, (contentHash >> 40) & 0XFFFF, contentHash);
    return path::join(_casPath, casFilePath);
}

auto up::ResourceLoader::_openFile(zstring_view filename) const -> Stream {
    if (filename.empty()) {
        return {};
    }

    return FileSystem::openRead(filename);
}

auto up::ResourceLoader::openAsset(ResourceId id) const -> Stream { return _openFile(assetPath(id)); }

auto up::ResourceLoader::openCAS(uint64 contentHash) const -> Stream { return _openFile(casPath(contentHash)); }

auto up::ResourceLoader::openAsset(string_view assetName) const -> Stream { return openAsset(assetHash(assetName)); }

auto up::ResourceLoader::openAsset(string_view assetName, string_view logicalName) const -> Stream {
    return openAsset(assetHash(assetName, logicalName));
}
