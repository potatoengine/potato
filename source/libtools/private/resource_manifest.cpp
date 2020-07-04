// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "resource_manifest.h"

#include "potato/format/format.h"
#include "potato/runtime/stream.h"

auto up::ResourceManifest::findHash(ResourceId id) const noexcept -> uint64 {
    for (auto const& record : _records) {
        if (record.id == id) {
            return record.hash;
        }
    }

    return 0;
}

auto up::ResourceManifest::findFilename(ResourceId id) const noexcept -> zstring_view {
    for (auto const& record : _records) {
        if (record.id == id) {
            return record.filename;
        }
    }

    return {};
}

bool up::ResourceManifest::parseManifest(Stream& stream) { return false; }

bool up::ResourceManifest::writeManifest(Stream& stream) const {
    if (!stream) {
        return false;
    }

    format_to(stream,
        "# Potato Manifest\n"
        ".version={}\n"
        ":ID|HASH|NAME\n",
        version);

    for (auto const& record : _records) {
        format_to(stream, "{}|{}|{}\n", record.id, record.hash, record.filename);
    }

    return true;
}

void up::ResourceManifest::addRecord(ResourceId id, uint64 hash, string filename) {
    for (auto& record : _records) {
        if (record.id == id) {
            record.hash = hash;
            record.filename = std::move(filename);
            return;
        }
    }

    _records.push_back({id, hash, std::move(filename)});
}
