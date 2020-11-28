// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_loader.h"
#include "filesystem.h"
#include "path.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"

up::ResourceLoader::ResourceLoader() : _logger("ResourceLoader") {}

up::ResourceLoader::~ResourceLoader() = default;

auto up::ResourceLoader::_assetPath(ResourceId id) const -> string {
    for (auto const& record : _manifest.records()) {
        if (record.logicalId == id) {
            return casPath(record.hash);
        }
    }
    return {};
}

auto up::ResourceLoader::_assetPath(ResourceId id, string_view logicalName) const -> string {
    auto const hash = hash_value(logicalName);
    for (auto const& record : _manifest.records()) {
        if (record.rootId == id && record.logicalName == hash) {
            return casPath(record.hash);
        }
    }
    return {};
}

auto up::ResourceLoader::_assetHash(string_view assetName) const -> ResourceId {
    return ResourceId{hash_value<fnv1a>(assetName)};
}

auto up::ResourceLoader::_assetHash(string_view assetName, string_view logicalName) const -> ResourceId {
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
    format_append(
        casFilePath,
        "{:02X}/{:04X}/{:016X}.bin",
        (contentHash >> 56) & 0xFF,
        (contentHash >> 40) & 0XFFFF,
        contentHash);
    return path::join(_casPath, casFilePath);
}

auto up::ResourceLoader::_openFile(zstring_view filename) const -> Stream {
    if (filename.empty()) {
        return {};
    }

    return fs::openRead(filename);
}

auto up::ResourceLoader::openAsset(ResourceId id) const -> Stream {
    return _openFile(_assetPath(id));
}

auto up::ResourceLoader::openCAS(uint64 contentHash) const -> Stream {
    return _openFile(casPath(contentHash));
}

auto up::ResourceLoader::translate(string_view assetName) const -> ResourceId {
    return ResourceId{hash_value<fnv1a>(assetName)};
}

auto up::ResourceLoader::openAsset(string_view assetName) const -> Stream {
    return openAsset(_assetHash(assetName));
}

auto up::ResourceLoader::openAsset(string_view assetName, string_view logicalName) const -> Stream {
    return openAsset(_assetHash(assetName, logicalName));
}

auto up::ResourceLoader::loadAsset(ResourceId id, string_view logicalName, string_view type) -> rc<Resource> {
    ResourceLoaderBackend* const backend = _findBackend(type);
    UP_ASSERT(backend != nullptr, "Unknown backend `{}`", type);

    ResourceManifest::Record const* const record = _findRecord(id, logicalName, type);
    if (record == nullptr) {
        _logger.error("Failed to find asset `{}:{}` ({})", id, logicalName, type);
        return nullptr;
    }

    string filename = casPath(record->hash);

    Stream stream = fs::openRead(filename);
    if (!stream) {
        _logger.error("Unknown asset `{}:{}` ({}) from `{}`", id, logicalName, type, filename);
        return nullptr;
    }

    auto resource = backend->loadFromStream(std::move(stream));
    if (!resource) {
        _logger.error("Load failed for asset `{}:{}` ({}) from `{}`", id, logicalName, type, filename);
        return nullptr;
    }

    return resource;
}

void up::ResourceLoader::addBackend(box<ResourceLoaderBackend> backend) {
    UP_ASSERT(backend != nullptr);

    _backends.push_back(std::move(backend));
}

auto up::ResourceLoader::_findRecord(ResourceId id, string_view logicalName, string_view type) const
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

auto up::ResourceLoader::_findBackend(string_view type) const -> ResourceLoaderBackend* {
    for (box<ResourceLoaderBackend> const& backend : _backends) {
        if (backend->typeName() == type) {
            return backend.get();
        }
    }
    return nullptr;
}
