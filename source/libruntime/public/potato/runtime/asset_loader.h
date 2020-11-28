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
    class AssetLoader;

    class Resource : public shared<Resource> {
    public:
        virtual ~Resource() = default;
    };

    class AssetLoaderBackend {
    public:
        virtual ~AssetLoaderBackend() = default;

        virtual zstring_view typeName() const noexcept = 0;
        virtual rc<Resource> loadFromStream(Stream stream, AssetLoader& AssetLoader) = 0;
    };

    class AssetLoader {
    public:
        UP_RUNTIME_API AssetLoader();
        UP_RUNTIME_API ~AssetLoader();

        ResourceManifest& manifest() noexcept { return _manifest; }
        void setCasPath(string path) { _casPath = std::move(path); }

        UP_RUNTIME_API ResourceId translate(string_view assetName, string_view logicalName = {}) const;

        template <typename ResourceT>
        rc<ResourceT> loadAssetSync(ResourceId id) {
            zstring_view constexpr typeName = ResourceT::resourceType;
            return rc<ResourceT>(static_cast<ResourceT*>(loadAssetSync(id, typeName).release()));
        }
        UP_RUNTIME_API rc<Resource> loadAssetSync(ResourceId id, string_view type = {});

        UP_RUNTIME_API void registerBackend(box<AssetLoaderBackend> backend);

    private:
        string _makeCasPath(uint64 contentHash) const;
        ResourceManifest::Record const* _findRecord(ResourceId logicalId) const;
        AssetLoaderBackend* _findBackend(string_view type) const;

        vector<box<AssetLoaderBackend>> _backends;
        ResourceManifest _manifest;
        string _casPath;
        Logger _logger;
    };
} // namespace up
