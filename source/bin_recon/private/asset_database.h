// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/format/erased.h"
#include "potato/posql/posql.h"
#include "potato/runtime/asset.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/generator.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/unique_resource.h"
#include "potato/spud/vector.h"

namespace up {
    class Stream;
    class ResourceManifest;

    class AssetDatabase {
    public:
        struct Dependency {
            zstring_view path;
            uint64 contentHash = 0;
        };

        struct Output {
            zstring_view name;
            zstring_view type;
            AssetId logicalAssetId = AssetId::Invalid;
            uint64 contentHash = 0;
        };

        struct Imported {
            UUID uuid;
            string sourcePath;
            string importerName;
            string assetType;
            uint64 importerRevision = 0;
            uint64 sourceContentHash = 0;
        };

        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 15;

        AssetDatabase() = default;
        ~AssetDatabase();

        AssetDatabase(AssetDatabase const&) = delete;
        AssetDatabase& operator=(AssetDatabase const&) = delete;

        auto pathToUuid(string_view path) noexcept -> UUID;
        auto uuidToPath(UUID const& uuid) noexcept -> string;

        static AssetId createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept;

        Imported findRecordByUuid(UUID const& uuid);
        generator<zstring_view const> collectAssetPathsByFolder(zstring_view folder);
        generator<zstring_view const> collectAssetPaths();

        generator<Dependency const> assetDependencies(UUID const& uuid);
        generator<Output const> assetOutputs(UUID const& uuid);

        bool insertRecord(Imported const& record);
        bool deleteRecordByUuid(UUID const& uuid);

        void addDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash);
        void addOutput(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash);

        bool open(zstring_view filename);
        bool close();

        void generateManifest(erased_writer writer);

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        Database _db;
        Statement _queryAssetsStmt;
        Statement _queryDependenciesStmt;
        Statement _queryOutputsStmt;
        Statement _queryAssetByUuidStmt;
        Statement _queryAssetBySourcePathStmt;
        Statement _insertAssetStmt;
        Statement _insertOutputStmt;
        Statement _insertDependencyStmt;
        Statement _deleteAssetStmt;
        Statement _clearOutputsStmt;
        Statement _clearDependenciesStmt;
    };
} // namespace up
