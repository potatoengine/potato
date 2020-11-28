// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_loader.h"
#include "asset.h"
#include "filesystem.h"
#include "path.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

#include <Tracy.hpp>

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

auto up::AssetLoader::debugName(ResourceId logicalId) const noexcept -> zstring_view {
    for (auto const& record : _manifest.records()) {
        if (record.logicalId == logicalId) {
            return record.filename;
        }
    }
    return {};
}

auto up::AssetLoader::loadAssetSync(ResourceId id, string_view type) -> rc<Asset> {
    ZoneScopedN("Load Asset Synchronous");

    AssetLoaderBackend* const backend = _findBackend(type);
    UP_ASSERT(backend != nullptr, "Unknown backend `{}`", type);

    ResourceManifest::Record const* const record = _findRecord(id);
    if (record == nullptr) {
        _logger.error("Failed to find asset `{}` ({})", id, type);
        return nullptr;
    }

    if (record->type != type) {
        _logger.error(
            "Invalid type for asset `{}` [{}:{}] ({}, expected {})",
            id,
            record->filename,
            record->logicalName,
            record->type,
            type);
        return nullptr;
    }

    string filename = _makeCasPath(record->hash);

    Stream stream = fs::openRead(filename);
    if (!stream) {
        _logger.error(
            "Unknown asset `{}` [{}:{}] ({}) from `{}`",
            id,
            record->filename,
            record->logicalName,
            type,
            filename);
        return nullptr;
    }

    AssetLoadContext const ctx{.id = id, .stream = stream, .loader = *this};

    auto asset = backend->loadFromStream(ctx);

    stream.close();

    if (!asset) {
        _logger.error(
            "Load failed for asset `{}` [{}:{}] ({}) from `{}`",
            id,
            record->filename,
            record->logicalName,
            type,
            filename);
        return nullptr;
    }

    return asset;
}

void up::AssetLoader::registerBackend(box<AssetLoaderBackend> backend) {
    UP_ASSERT(backend != nullptr);

    _backends.push_back(std::move(backend));
}

auto up::AssetLoader::_findRecord(ResourceId logicalId) const -> ResourceManifest::Record const* {
    for (auto const& record : _manifest.records()) {
        if (record.logicalId == logicalId) {
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
