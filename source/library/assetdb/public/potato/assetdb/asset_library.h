// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include "asset_record.h"
#include "potato/foundation/vector.h"
#include <unordered_map>
#include <iosfwd>

namespace up {
    namespace fs {
        class Stream;
        class Stream;
    } // namespace fs

    class AssetLibrary {
    public:
        AssetLibrary() = default;
        UP_ASSETDB_API ~AssetLibrary();

        AssetLibrary(AssetLibrary const&) = delete;
        AssetLibrary& operator=(AssetLibrary const&) = delete;

        UP_ASSETDB_API auto pathToAssetId(string_view path) const -> AssetId;
        UP_ASSETDB_API auto assetIdToPath(AssetId assetId) const -> string_view;

        UP_ASSETDB_API AssetImportRecord const* findRecord(AssetId assetId) const;

        UP_ASSETDB_API bool insertRecord(AssetImportRecord record);

        UP_ASSETDB_API bool serialize(fs::Stream& stream) const;
        UP_ASSETDB_API bool deserialize(fs::Stream& stream);

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        std::unordered_map<AssetId, AssetImportRecord, HashAssetId> _assets;
    };
} // namespace up