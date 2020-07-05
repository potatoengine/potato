// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_loader.h"
#include "filesystem.h"
#include "path.h"
#include "stream.h"

#include "potato/format/format.h"
#include "potato/spud/fixed_string_writer.h"

up::ResourceLoader::~ResourceLoader() = default;

auto up::ResourceLoader::assetPath(ResourceId id) const -> string {
    for (auto const& record : _manifest.records()) {
        if (record.id == id) {
            return casPath(record.hash);
        }
    }
    return {};
}

auto up::ResourceLoader::assetPath(ResourceId id, string_view logicalName) const -> string { return {}; }

auto up::ResourceLoader::casPath(uint64 contentHash) const -> string {
    fixed_string_writer<32> casPath;
    format_append(casPath, "{:02X}/{:04X}/{:016X}.bin", (contentHash >> 56) & 0xFF, (contentHash >> 40) & 0XFFFF, contentHash);
    return path::join(_casPath, "cache", casPath);
}

auto up::ResourceLoader::openFile(zstring_view filename) const -> Stream {
    if (filename.empty()) {
        return {};
    }

    auto const fullPath = path::join(_casPath, filename);
    return _fileSystem->openRead(fullPath);
}

auto up::ResourceLoader::openAsset(ResourceId id) const -> Stream { return openFile(assetPath(id)); }

auto up::ResourceLoader::openAsset(ResourceId id, string_view logicalName) const -> Stream { return openFile(assetPath(id, logicalName)); }

auto up::ResourceLoader::openCAS(uint64 contentHash) const -> Stream { return openFile(casPath(contentHash)); }
