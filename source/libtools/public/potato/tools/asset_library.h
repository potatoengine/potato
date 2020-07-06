// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"

#include "potato/format/erased.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class ResourceManifest;

    class AssetLibrary {
    public:
        struct Dependency {
            string path;
            uint64 contentHash = 0;
        };

        struct Output {
            string name;
            AssetId logicalAssetId = AssetId::Invalid;
            uint64 contentHash = 0;
        };

        struct Imported {
            AssetId assetId = AssetId::Invalid;
            string sourcePath;
            string importerName;
            uint64 importerRevision = 0;
            uint64 sourceContentHash = 0;

            vector<Dependency> dependencies;
            vector<Output> outputs;
        };

        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 11;

        AssetLibrary() = default;
        UP_TOOLS_API ~AssetLibrary();

        AssetLibrary(AssetLibrary const&) = delete;
        AssetLibrary& operator=(AssetLibrary const&) = delete;

        UP_TOOLS_API auto pathToAssetId(string_view path) const -> AssetId;
        UP_TOOLS_API auto assetIdToPath(AssetId assetId) const -> string_view;

        UP_TOOLS_API Imported const* findRecord(AssetId assetId) const;

        UP_TOOLS_API bool insertRecord(Imported record);

        UP_TOOLS_API bool serialize(Stream& stream) const;
        UP_TOOLS_API bool deserialize(Stream& stream);

        UP_TOOLS_API void generateManifest(erased_writer writer) const;

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        vector<Imported> _records;
    };
} // namespace up
