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

    class Asset : public shared<Asset> {
    public:
        explicit Asset(ResourceId id) noexcept : _id(id) {}
        virtual ~Asset() = default;

        ResourceId assetId() const noexcept { return _id; }

    private:
        ResourceId _id{};
    };

    struct AssetLoadContext {
        ResourceId id{};
        Stream& stream;
        AssetLoader& loader;
    };

    class AssetLoaderBackend {
    public:
        virtual ~AssetLoaderBackend() = default;

        virtual zstring_view typeName() const noexcept = 0;
        virtual rc<Asset> loadFromStream(AssetLoadContext const& ctx) = 0;
    };

    class AssetLoader {
    public:
        UP_RUNTIME_API AssetLoader();
        UP_RUNTIME_API ~AssetLoader();

        ResourceManifest& manifest() noexcept { return _manifest; }
        void setCasPath(string path) { _casPath = std::move(path); }

        UP_RUNTIME_API ResourceId translate(string_view assetName, string_view logicalName = {}) const;
        UP_RUNTIME_API zstring_view debugName(ResourceId logicalId) const noexcept;

        template <typename AssetT>
        rc<AssetT> loadAssetSync(ResourceId id) {
            zstring_view constexpr typeName = AssetT::assetTypeName;
            return rc<AssetT>(static_cast<AssetT*>(loadAssetSync(id, typeName).release()));
        }
        UP_RUNTIME_API rc<Asset> loadAssetSync(ResourceId id, string_view type = {});

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
