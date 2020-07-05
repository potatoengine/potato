// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "resource_manifest.h"

#include "potato/spud/string.h"

namespace up {
    class FileSystem;
    class Stream;

    class ResourceLoader {
    public:
        UP_RUNTIME_API explicit ResourceLoader(FileSystem& fileSystem) : _fileSystem(&fileSystem) {}
        UP_RUNTIME_API ~ResourceLoader();

        ResourceManifest& manifest() noexcept { return _manifest; }
        void setCasPath(string path) { _casPath = std::move(path); }

        UP_RUNTIME_API string assetPath(ResourceId id) const;
        UP_RUNTIME_API string assetPath(ResourceId id, string_view logicalName) const;

        UP_RUNTIME_API string casPath(uint64 contentHash) const;

        UP_RUNTIME_API Stream openFile(zstring_view filename) const;
        UP_RUNTIME_API Stream openAsset(ResourceId id) const;
        UP_RUNTIME_API Stream openAsset(ResourceId id, string_view logicalName) const;
        UP_RUNTIME_API Stream openCAS(uint64 contentHash) const;

    private:
        FileSystem* _fileSystem = nullptr;
        ResourceManifest _manifest;
        string _casPath;
    };
} // namespace up
