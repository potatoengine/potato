// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_loader.h"
#include "filesystem.h"
#include "path.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

up::AssetLoader::AssetLoader() : _logger("AssetLoader") {}

up::AssetLoader::~AssetLoader() = default;

auto up::AssetLoader::translate(string_view assetName, string_view logicalName) const -> ResourceId {
    fnv1a engine;
    hash_append(engine, assetName);
    if (!logicalName.empty()) {
        hash_append(engine, ":"_zsv);
        hash_append(engine, logicalName);
    }
    return ResourceId{engine.finalize()};
}

auto up::AssetLoader::loadAsset(ResourceId id, string_view logicalName, string_view type) -> rc<Resource> {
    AssetLoaderBackend* const backend = _findBackend(type);
    UP_ASSERT(backend != nullptr, "Unknown backend `{}`", type);

    ResourceManifest::Record const* const record = _findRecord(id, logicalName, type);
    if (record == nullptr) {
        _logger.error("Failed to find asset `{}:{}` ({})", id, logicalName, type);
        return nullptr;
    }

    string filename = _makeCasPath(record->hash);

    Stream stream = fs::openRead(filename);
    if (!stream) {
        _logger.error("Unknown asset `{}:{}` ({}) from `{}`", id, logicalName, type, filename);
        return nullptr;
    }

    auto resource = backend->loadFromStream(std::move(stream), *this);
    if (!resource) {
        _logger.error("Load failed for asset `{}:{}` ({}) from `{}`", id, logicalName, type, filename);
        return nullptr;
    }

    return resource;
}

void up::AssetLoader::addBackend(box<AssetLoaderBackend> backend) {
    UP_ASSERT(backend != nullptr);

    _backends.push_back(std::move(backend));
}

auto up::AssetLoader::_findRecord(ResourceId id, string_view logicalName, string_view type) const
    -> ResourceManifest::Record const* {
    // find the correct record
    uint64 const logicalHash = hash_value(logicalName);
    for (auto const& record : _manifest.records()) {
        if (record.rootId == id && record.logicalName == logicalHash && record.type == type) {
            return &record;
        }
    }
    return nullptr;
}

auto up::AssetLoader::_findBackend(string_view type) const -> AssetLoaderBackend* {
    for (box<AssetLoaderBackend> const& backend : _backends) {
        if (backend->typeName() == type) {
            return backend.get();
        }
    }
    return nullptr;
}

auto up::AssetLoader::_makeCasPath(uint64 contentHash) const -> string {
    char casFilePath[32] = {
        0,
    };
    format_to(
        casFilePath,
        "{:02X}/{:04X}/{:016X}.bin",
        (contentHash >> 56) & 0xFF,
        (contentHash >> 40) & 0XFFFF,
        contentHash);
    return path::join(_casPath, casFilePath);
}
