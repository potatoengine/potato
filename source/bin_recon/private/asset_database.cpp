// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_database.h"

#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/string_writer.h"

up::AssetDatabase::~AssetDatabase() = default;

auto up::AssetDatabase::pathToUuid(string_view path) noexcept -> UUID {
    for (auto const& [uuid] : _queryUuidBySourcePathStmt.query<UUID>(path)) {
        return uuid;
    }

    return {};
}

auto up::AssetDatabase::uuidToPath(UUID const& uuid) noexcept -> string {
    for (auto const& [sourcePath] : _querySourcePathByUuuidStmt.query<zstring_view>(uuid)) {
        return string{sourcePath};
    }

    return {};
}

auto up::AssetDatabase::collectAssetPathsByFolder(zstring_view folder) -> generator<zstring_view const> {
    for (zstring_view sourcePath : collectAssetPaths()) {
        if (path::isParentOf(folder, sourcePath)) {
            co_yield sourcePath;
        }
    }
}

auto up::AssetDatabase::collectAssetPaths() -> generator<zstring_view const> {
    for (auto const& [uuid, sourcePath, sourceHash, assetType, importName, importVer] :
         _queryAssetsStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>()) {
        co_yield sourcePath;
    }
}

auto up::AssetDatabase::assetDependencies(UUID const& uuid) -> generator<Dependency const> {
    for (auto const& [path, hash] : _queryDependenciesStmt.query<zstring_view, uint64>(uuid)) {
        co_yield Dependency{.path = string{path}, .contentHash = hash};
    }
}

auto up::AssetDatabase::assetOutputs(UUID const& uuid) -> generator<Output const> {
    for (auto const& [id, name, type, hash] :
         _queryOutputsStmt.query<AssetId, zstring_view, zstring_view, uint64>(uuid)) {
        co_yield Output{.name = name, .type = type, .logicalAssetId = id, .contentHash = hash};
    }
}

auto up::AssetDatabase::createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept -> AssetId {
    uint64 hash = hash_value(uuid);
    if (!logicalName.empty()) {
        hash = hash_combine(hash, hash_value(logicalName));
    }
    return static_cast<AssetId>(hash);
}

void up::AssetDatabase::createAsset(UUID const& uuid, string_view sourcePath, uint64 sourceHash) {
    [[maybe_unused]] auto const rc = _insertAssetStmt.execute(uuid, sourcePath, sourceHash);
    UP_ASSERT(rc == SqlResult::Ok);
}

bool up::AssetDatabase::checkAssetUpToDate(
    UUID const& uuid,
    string_view importerName,
    uint64 importerVersion,
    uint64 sourceHash) {
    auto [result] = _queryAssetUpToDateStmt.queryOne<bool>(sourceHash, importerName, importerVersion, uuid);
    return result;
}

void up::AssetDatabase::updateAssetPre(
    UUID const& uuid,
    string_view importerName,
    string_view assetType,
    uint64 importerVersion) {
    [[maybe_unused]] auto const rc = _updateAssetPreStmt.execute(importerName, assetType, importerVersion, uuid);
    UP_ASSERT(rc == SqlResult::Ok);
}

void up::AssetDatabase::updateAssetPost(UUID const& uuid, bool success) {
    [[maybe_unused]] auto rc = _updateAssetPostStmt.execute(success ? "IMPORTED"_sv : "FAILED"_sv, uuid);
    UP_ASSERT(rc == SqlResult::Ok);
    rc = _clearDependenciesStmt.execute(uuid);
    UP_ASSERT(rc == SqlResult::Ok);
    rc = _clearOutputsStmt.execute(uuid);
    UP_ASSERT(rc == SqlResult::Ok);
}

bool up::AssetDatabase::deleteAsset(UUID const& uuid) {
    [[maybe_unused]] auto const rc = _deleteAssetStmt.execute(uuid);
    UP_ASSERT(rc == SqlResult::Ok);

    return true;
}

void up::AssetDatabase::addDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash) {
    [[maybe_unused]] auto const rc = _insertDependencyStmt.execute(uuid, outputPath, outputHash);
    UP_ASSERT(rc == SqlResult::Ok);
}

void up::AssetDatabase::addOutput(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash) {
    [[maybe_unused]] auto const rc =
        _insertOutputStmt.execute(uuid, createLogicalAssetId(uuid, name), name, assetType, outputHash);
    UP_ASSERT(rc == SqlResult::Ok);
}

bool up::AssetDatabase::open(zstring_view filename) {
    // open the database
    if (_db.open(filename.c_str()) != SqlResult::Ok) {
        return false;
    }

    // ensure the asset database has the correct version
    {
        if (_db.execute("CREATE TABLE IF NOT EXISTS version(version INTEGER NOT NULL);") != SqlResult::Ok) {
            return false;
        }
        auto dbVersionQueryStmt = _db.prepare("SELECT version FROM version");
        int64 dbVersion = 0;
        for (auto const& [versionResult] : dbVersionQueryStmt.query<int64>()) {
            dbVersion = versionResult;
        }
        if (dbVersion != version) {
            (void)_db.execute("DROP TABLE assets");
            (void)_db.execute("DROP TABLE dependencies");
            (void)_db.execute("DROP TABLE outputs");
            (void)_db.execute("DELETE FROM version");
            auto dbVersionUpdateStmt = _db.prepare("INSERT INTO version (version) VALUES(?)");
            if (dbVersionUpdateStmt.execute(version) != SqlResult::Ok) {
                return false;
            }
        }
    }

    // ensure the table is created
    if (_db.execute("CREATE TABLE IF NOT EXISTS assets "
                    "(uuid TEXT PRIMARY KEY, status TEXT, "
                    "source_path TEXT, source_hash INTEGER, asset_type TEXT, "
                    "importer_name TEXT, importer_revision INTEGER);\n"

                    "CREATE TABLE IF NOT EXISTS outputs "
                    "(uuid TEXT, output_id INTEGER, name TEXT, type TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid));\n"

                    "CREATE TABLE IF NOT EXISTS dependencies "
                    "(uuid TEXT, db_path TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid));\n") != SqlResult::Ok) {
        return false;
    }

    // ensure any left-over assets are transitioned to failed status
    (void)_db.execute("UPDATE assets SET status='FAILED' WHERE status<>'IMPORTED'");

    // create our prepared statements for later use
    _queryAssetsStmt =
        _db.prepare("SELECT uuid, source_path, source_hash, asset_type, importer_name, importer_revision FROM assets");
    _queryAssetUpToDateStmt = _db.prepare(
        "SELECT (source_hash=? AND importer_name=? AND importer_revision=?) AS up_to_date FROM assets WHERE uuid=?");
    _queryDependenciesStmt = _db.prepare("SELECT db_path, hash FROM dependencies WHERE uuid=?");
    _queryOutputsStmt = _db.prepare("SELECT output_id, name, type, hash FROM outputs WHERE uuid=?");
    _queryUuidBySourcePathStmt = _db.prepare("SELECT uuid FROM assets WHERE source_path=?");
    _querySourcePathByUuuidStmt = _db.prepare("SELECT source_path FROM assets WHERE uuid=?");
    _insertAssetStmt = _db.prepare(
        "INSERT INTO assets "
        "(uuid, source_path, source_hash, status) VALUES(?, ?, ?, 'NEW')"
        "ON CONFLICT (uuid) DO UPDATE SET source_path=excluded.source_path, source_hash=excluded.source_hash");
    _updateAssetPreStmt = _db.prepare(
        "UPDATE assets SET importer_name=?, asset_type=?, importer_revision=?, status='PENDING' WHERE uuid=?");
    _updateAssetPostStmt = _db.prepare("UPDATE assets SET status=? WHERE uuid=?");
    _insertOutputStmt = _db.prepare("INSERT INTO outputs (uuid, output_id, name, type, hash) VALUES(?, ?, ?, ?, ?)");
    _insertDependencyStmt = _db.prepare("INSERT INTO dependencies (uuid, db_path, hash) VALUES(?, ?, ?)");
    _deleteAssetStmt = _db.prepare("DELETE FROM assets WHERE uuid=?");
    _clearOutputsStmt = _db.prepare("DELETE FROM outputs WHERE uuid=?");
    _clearDependenciesStmt = _db.prepare("DELETE FROM dependencies WHERE uuid=?");

    return true;
}

bool up::AssetDatabase::close() {
    _db.close();
    return true;
}

void up::AssetDatabase::generateManifest(erased_writer writer) {
    format_to(writer, "# Potato Manifest\n");
    format_to(writer, ".version={}\n", ResourceManifest::version);
    format_to(
        writer,
        ":{}|{}|{}|{}|{}|{}\n",
        ResourceManifest::columnUuid,
        ResourceManifest::columnLogicalId,
        ResourceManifest::columnLogicalName,
        ResourceManifest::columnContentType,
        ResourceManifest::columnContentHash,
        ResourceManifest::columnDebugName);

    string_writer fullName;

    for (auto const& [uuid, sourcePath, sourceHash, assetType, importName, importVer] :
         _queryAssetsStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>()) {
        format_to(writer, "{}|||{}||{}\n", uuid, assetType, sourcePath);
    }

    for (auto const& [uuid, sourcePath, sourceHash, assetType, importName, importVer] :
         _queryAssetsStmt.query<UUID, zstring_view, uint64, zstring_view, zstring_view, uint64>()) {
        for (auto const& [logicalId, logicalName, outputType, outputHash] :
             _queryOutputsStmt.query<AssetId, zstring_view, zstring_view, uint64>(uuid)) {
            fullName.clear();
            fullName.append(sourcePath);

            if (!logicalName.empty()) {
                format_append(fullName, ":{}", logicalName);
            }

            format_to(
                writer,
                "{}|{:016X}|{}|{}|{:016X}|{}\n",
                uuid,
                createLogicalAssetId(uuid, logicalName),
                logicalName,
                outputType,
                outputHash,
                fullName);
        }
    }
}
