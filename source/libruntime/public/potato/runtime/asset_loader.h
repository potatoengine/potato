// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "logger.h"

#include "potato/spud/box.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class Asset;
    class AssetLoader;
    class ResourceManifest;

    enum class AssetId : uint64;

    struct AssetLoadContext {
        AssetId id{};
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

        ResourceManifest const* manifest() const noexcept { return _manifest.get(); }

        UP_RUNTIME_API void bindManifest(box<ResourceManifest> manifest, string casPath);

        UP_RUNTIME_API AssetId translate(string_view assetName, string_view logicalName = {}) const;
        UP_RUNTIME_API zstring_view debugName(AssetId logicalId) const noexcept;

        template <typename AssetT>
        rc<AssetT> loadAssetSync(AssetId id) {
            zstring_view constexpr typeName = AssetT::assetTypeName;
            return rc<AssetT>(static_cast<AssetT*>(loadAssetSync(id, typeName).release()));
        }
        UP_RUNTIME_API rc<Asset> loadAssetSync(AssetId id, string_view type = {});

        UP_RUNTIME_API void registerBackend(box<AssetLoaderBackend> backend);

    private:
        string _makeCasPath(uint64 contentHash) const;
        AssetLoaderBackend* _findBackend(string_view type) const;

        vector<box<AssetLoaderBackend>> _backends;
        box<ResourceManifest> _manifest;
        string _casPath;
        Logger _logger;
    };
} // namespace up
