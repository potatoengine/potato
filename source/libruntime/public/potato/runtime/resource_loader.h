// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "logger.h"
#include "resource_manifest.h"

#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class ResourceLoader;

    class Resource : public shared<Resource> {
    public:
        virtual ~Resource() = default;
    };

    class ResourceLoaderBackend {
    public:
        virtual ~ResourceLoaderBackend() = default;

        virtual zstring_view typeName() const noexcept = 0;
        virtual rc<Resource> loadFromStream(Stream stream, ResourceLoader& resourceLoader) = 0;
    };

    class ResourceLoader {
    public:
        UP_RUNTIME_API ResourceLoader();
        UP_RUNTIME_API ~ResourceLoader();

        ResourceManifest& manifest() noexcept { return _manifest; }
        void setCasPath(string path) { _casPath = std::move(path); }

        UP_RUNTIME_API string casPath(uint64 contentHash) const;

        UP_RUNTIME_API Stream openAsset(ResourceId id) const;
        UP_RUNTIME_API Stream openAsset(string_view assetName) const;
        UP_RUNTIME_API Stream openAsset(string_view assetName, string_view logicalName) const;
        UP_RUNTIME_API Stream openCAS(uint64 contentHash) const;

        UP_RUNTIME_API ResourceId translate(string_view assetName) const;

        template <typename ResourceT>
        rc<ResourceT> loadAsset(ResourceId id, string_view logicalName = {}) {
            zstring_view constexpr typeName = ResourceT::resourceType;
            return rc<ResourceT>(static_cast<ResourceT*>(loadAsset(id, logicalName, typeName).release()));
        }
        UP_RUNTIME_API rc<Resource> loadAsset(ResourceId id, string_view logicalName = {}, string_view type = {});

        UP_RUNTIME_API void addBackend(box<ResourceLoaderBackend> backend);

    private:
        string _assetPath(ResourceId id) const;
        string _assetPath(ResourceId id, string_view logicalName) const;
        ResourceId _assetHash(string_view assetName) const;
        ResourceId _assetHash(string_view assetName, string_view logicalName) const;
        ResourceManifest::Record const* _findRecord(ResourceId id, string_view logicalName, string_view type) const;
        ResourceLoaderBackend* _findBackend(string_view type) const;
        Stream _openFile(zstring_view filename) const;

        vector<box<ResourceLoaderBackend>> _backends;
        ResourceManifest _manifest;
        string _casPath;
        Logger _logger;
    };
} // namespace up
