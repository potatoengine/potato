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

        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 16;

        AssetDatabase() = default;
        ~AssetDatabase();

        AssetDatabase(AssetDatabase const&) = delete;
        AssetDatabase& operator=(AssetDatabase const&) = delete;

        auto pathToUuid(string_view path) noexcept -> UUID;
        auto uuidToPath(UUID const& uuid) noexcept -> string;

        static AssetId createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept;

        generator<zstring_view const> collectAssetPathsByFolder(zstring_view folder);
        generator<zstring_view const> collectAssetPaths();

        generator<Dependency const> assetDependencies(UUID const& uuid);
        generator<Output const> assetOutputs(UUID const& uuid);

        void createAsset(UUID const& uuid, zstring_view sourcePath, uint64 sourceHash);
        bool checkAssetUpToDate(UUID const& uuid, string_view importerName, uint64 importerVersion, uint64 sourceHash);
        void updateAssetPre(
            UUID const& uuid,
            zstring_view importerName,
            zstring_view assetType,
            uint64 importerVersion);
        void updateAssetPost(UUID const& uuid, bool success);

        bool deleteAsset(UUID const& uuid);

        void clearDependencies(UUID const& uuid);
        void clearOutputs(UUID const& uuid);

        void addDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash);
        void addOutput(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash);

        bool open(zstring_view filename);
        bool close();

        void generateManifest(erased_writer writer);

        template <typename Fn>
        void transact(Fn&& fn) {
            auto tx = _db.begin();
            fn(*this, tx);
            tx.commit();
        }

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        Database _db;
        Statement _queryAssetsStmt;
        Statement _queryAssetUpToDateStmt;
        Statement _queryDependenciesStmt;
        Statement _queryOutputsStmt;
        Statement _queryUuidBySourcePathStmt;
        Statement _querySourcePathByUuuidStmt;
        Statement _insertAssetStmt;
        Statement _updateAssetPreStmt;
        Statement _updateAssetPostStmt;
        Statement _insertOutputStmt;
        Statement _insertDependencyStmt;
        Statement _deleteAssetStmt;
        Statement _clearOutputsStmt;
        Statement _clearDependenciesStmt;
    };
} // namespace up
