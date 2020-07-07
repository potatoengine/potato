// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "resource_manifest.h"

#include "potato/spud/string.h"

namespace up {
    class Stream;

    class ResourceLoader {
    public:
        UP_RUNTIME_API ResourceLoader();
        UP_RUNTIME_API ~ResourceLoader();

        ResourceManifest& manifest() noexcept { return _manifest; }
        void setCasPath(string path) { _casPath = std::move(path); }

        UP_RUNTIME_API string assetPath(ResourceId id) const;
        UP_RUNTIME_API string assetPath(ResourceId id, string_view logicalName) const;

        UP_RUNTIME_API ResourceId assetHash(string_view assetName) const;
        UP_RUNTIME_API ResourceId assetHash(string_view assetName, string_view logicalName) const;

        UP_RUNTIME_API string casPath(uint64 contentHash) const;

        UP_RUNTIME_API Stream openAsset(ResourceId id) const;
        UP_RUNTIME_API Stream openAsset(string_view assetName) const;
        UP_RUNTIME_API Stream openAsset(string_view assetName, string_view logicalName) const;
        UP_RUNTIME_API Stream openCAS(uint64 contentHash) const;

    private:
        Stream _openFile(zstring_view filename) const;

        ResourceManifest _manifest;
        string _casPath;
    };
} // namespace up
