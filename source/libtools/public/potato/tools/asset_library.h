// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "asset_record.h"
#include "common.h"

#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

#include <unordered_map>

namespace up {
    class Stream;
    class ResourceManifest;

    class AssetLibrary {
    public:
        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 5;

        AssetLibrary() = default;
        UP_TOOLS_API ~AssetLibrary();

        AssetLibrary(AssetLibrary const&) = delete;
        AssetLibrary& operator=(AssetLibrary const&) = delete;

        UP_TOOLS_API auto pathToAssetId(string_view path) const -> AssetId;
        UP_TOOLS_API auto assetIdToPath(AssetId assetId) const -> string_view;

        UP_TOOLS_API AssetImportRecord const* findRecord(AssetId assetId) const;

        UP_TOOLS_API bool insertRecord(AssetImportRecord record);

        UP_TOOLS_API bool serialize(Stream& stream) const;
        UP_TOOLS_API bool deserialize(Stream& stream);

        UP_TOOLS_API auto generateManifest() const -> ResourceManifest;

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        vector<AssetImportRecord> _records;
    };
} // namespace up
