// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/vector.h"
#include "grimm/library/common.h"
#include "grimm/library/asset_record.h"
#include <unordered_map>
#include <iosfwd>

namespace gm {
    class AssetLibrary {
    public:
        AssetLibrary() = default;
        GM_LIBRARY_API ~AssetLibrary();

        AssetLibrary(AssetLibrary const&) = delete;
        AssetLibrary& operator=(AssetLibrary const&) = delete;

        GM_LIBRARY_API auto pathToAssetId(string_view path) const -> AssetId;
        GM_LIBRARY_API auto assetIdToPath(AssetId assetId) const -> string_view;

        GM_LIBRARY_API AssetRecord const* findRecord(AssetId assetId) const;

        GM_LIBRARY_API bool insertRecord(AssetRecord record);

        GM_LIBRARY_API bool serialize(std::ostream& stream) const;
        GM_LIBRARY_API bool deserialize(std::istream& stream) const;

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        std::unordered_map<AssetId, AssetRecord, HashAssetId> _assets;
    };
} // namespace gm
