// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/format/erased.h"
#include "potato/posql/posql.h"
#include "potato/runtime/asset.h"
#include "potato/runtime/uuid.h"
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
            string path;
            uint64 contentHash = 0;
        };

        struct Output {
            string name;
            string type;
            AssetId logicalAssetId = AssetId::Invalid;
            uint64 contentHash = 0;
        };

        struct Imported {
            UUID uuid;
            string sourcePath;
            string importerName;
            uint64 importerRevision = 0;
            uint64 sourceContentHash = 0;

            vector<Dependency> dependencies;
            vector<Output> outputs;
        };

        static constexpr zstring_view typeName = "potato.asset.library"_zsv;
        static constexpr int version = 13;

        AssetDatabase() = default;
        ~AssetDatabase();

        AssetDatabase(AssetDatabase const&) = delete;
        AssetDatabase& operator=(AssetDatabase const&) = delete;

        auto pathToUuid(string_view path) const noexcept -> UUID;
        auto uuidToPath(UUID const& uuid) const noexcept -> string_view;

        static AssetId createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept;

        Imported const* findRecordByUuid(UUID const& uuid) const noexcept;
        Imported const* findRecordByFilename(zstring_view filename) const noexcept;

        bool insertRecord(Imported record);
        bool deleteSource(string_view sourcePath);

        bool open(zstring_view filename);
        bool close();

        void generateManifest(erased_writer writer) const;

    private:
        struct HashAssetId {
            constexpr uint64 operator()(AssetId assetId) const noexcept { return static_cast<uint64>(assetId); }
        };

        vector<Imported> _records;
        Database _db;
        Statement _insertAssetStmt;
        Statement _insertOutputStmt;
        Statement _insertDependencyStmt;
        Statement _deleteAssetStmt;
        Statement _clearOutputsStmt;
        Statement _clearDependenciesStmt;
    };
} // namespace up
