// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_loader.h"
#include "asset.h"
#include "filesystem.h"
#include "path.h"
#include "resource_manifest.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

#include <Tracy.hpp>

up::AssetLoader::AssetLoader() : _logger("AssetLoader") {}

up::AssetLoader::~AssetLoader() = default;

void up::AssetLoader::bindManifest(box<ResourceManifest> manifest, string casPath) {
    _manifest = std::move(manifest);
    _casPath = std::move(casPath);
}

auto up::AssetLoader::translate(string_view assetName, string_view logicalName) const -> AssetId {
    fnv1a engine;
    hash_append(engine, assetName);
    if (!logicalName.empty()) {
        hash_append(engine, ":"_zsv);
        hash_append(engine, logicalName);
    }
    return AssetId{engine.finalize()};
}

auto up::AssetLoader::debugName(AssetId logicalId) const noexcept -> zstring_view {
    auto const* record = _manifest != nullptr ? _manifest->findRecord(static_cast<uint64>(logicalId)) : nullptr;
    return record != nullptr ? record->filename : zstring_view{};
}

auto up::AssetLoader::loadAssetSync(AssetId id, string_view type) -> UntypedAssetHandle {
    ZoneScopedN("Load Asset Synchronous");

    AssetLoaderBackend* const backend = _findBackend(type);
    UP_ASSERT(backend != nullptr, "Unknown backend `{}`", type);

    ResourceManifest::Record const* const record =
        _manifest != nullptr ? _manifest->findRecord(static_cast<uint64>(id)) : nullptr;
    if (record == nullptr) {
        _logger.error("Failed to find asset `{}` ({})", id, type);
        return {};
    }

    if (record->type != type) {
        _logger.error(
            "Invalid type for asset `{}` [{}:{}] ({}, expected {})",
            id,
            record->filename,
            record->logicalName,
            record->type,
            type);
        return {};
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
        return {};
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
        return {};
    }

    return {static_cast<AssetId>(record->logicalId), std::move(asset)};
}

void up::AssetLoader::registerBackend(box<AssetLoaderBackend> backend) {
    UP_ASSERT(backend != nullptr);

    _backends.push_back(std::move(backend));
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
