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
    for (auto const& [uuid, sourcePath, sourceHash, assetType, importName, importVer] :
         _queryAssetBySourcePathStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>(
             path)) {
        return UUID::fromString(uuid);
    }

    return {};
}

auto up::AssetDatabase::uuidToPath(UUID const& uuid) noexcept -> string {
    char uuidStr[UUID::strLength] = {0};
    format_to(uuidStr, "{}", uuid);

    for (auto const& [uuid, sourcePath, sourceHash, assetType, importName, importVer] :
         _queryAssetByUuidStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>(uuidStr)) {
        return string{sourcePath};
    }

    return {};
}

auto up::AssetDatabase::findRecordByUuid(UUID const& uuid) -> Imported {
    char uuidStr[UUID::strLength] = {0};
    format_to(uuidStr, "{}", uuid);

    for (auto const& [uuid, southPath, sourceHash, assetType, importName, importVer] :
         _queryAssetByUuidStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>(uuidStr)) {
        Imported record{
            .uuid = UUID::fromString(uuid),
            .sourcePath = string{southPath},
            .importerName = string{importName},
            .assetType = string(assetType),
            .importerRevision = importVer,
            .sourceContentHash = sourceHash};

        for (auto const& [id, name, type, hash] :
             _queryOutputsStmt.query<AssetId, zstring_view, zstring_view, uint64>(uuidStr)) {
            record.outputs.push_back(
                Output{.name = string{name}, .type = string{type}, .logicalAssetId = id, .contentHash = hash});
        }

        return record;
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
    char uuidStr[UUID::strLength] = {0};
    format_to(uuidStr, "{}", uuid);

    for (auto const& [path, hash] : _queryDependenciesStmt.query<zstring_view, uint64>(uuidStr)) {
        co_yield Dependency{.path = string{path}, .contentHash = hash};
    }
}

auto up::AssetDatabase::createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept -> AssetId {
    uint64 hash = hash_value(uuid);
    if (!logicalName.empty()) {
        hash = hash_combine(hash, hash_value(logicalName));
    }
    return static_cast<AssetId>(hash);
}

bool up::AssetDatabase::insertRecord(Imported record) {
    char uuidStr[UUID::strLength] = {};
    format_to(uuidStr, "{}", record.uuid);

    // update database
    if (_insertAssetStmt) {
        auto tx = _db.begin();
        (void)_insertAssetStmt.execute(
            uuidStr,
            record.sourcePath.c_str(),
            record.sourceContentHash,
            record.assetType.c_str(),
            record.importerName.c_str(),
            record.importerRevision);

        (void)_clearOutputsStmt.execute(uuidStr);
        for (auto const& output : record.outputs) {
            (void)_insertOutputStmt
                .execute(uuidStr, output.logicalAssetId, output.name.c_str(), output.type.c_str(), output.contentHash);
        }

        (void)_clearDependenciesStmt.execute(uuidStr);

        tx.commit();
    }

    return true;
}

bool up::AssetDatabase::deleteRecordByUuid(UUID const& uuid) {
    char uuidStr[UUID::strLength] = {};
    format_to(uuidStr, "{}", uuid);

    // update database
    if (_deleteAssetStmt) {
        (void)_deleteAssetStmt.execute(uuidStr);
    }

    return true;
}

void up::AssetDatabase::addDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash) {
    char uuidStr[UUID::strLength] = {};
    format_to(uuidStr, "{}", uuid);

    (void)_insertDependencyStmt.execute(uuidStr, outputPath, outputHash);
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
                    "(uuid TEXT PRIMARY KEY, "
                    "source_db_path TEXT, source_hash INTEGER, asset_type TEXT, "
                    "importer_name TEXT, importer_revision INTEGER);\n"

                    "CREATE TABLE IF NOT EXISTS outputs "
                    "(uuid TEXT, output_id INTEGER, name TEXT, type TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid));\n"

                    "CREATE TABLE IF NOT EXISTS dependencies "
                    "(uuid TEXT, db_path TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid));\n") != SqlResult::Ok) {
        return false;
    }

    // create our prepared statements for later use
    _queryAssetsStmt = _db.prepare(
        "SELECT uuid, source_db_path, source_hash, asset_type, importer_name, importer_revision FROM assets");
    _queryDependenciesStmt = _db.prepare("SELECT db_path, hash FROM dependencies WHERE uuid=?");
    _queryOutputsStmt = _db.prepare("SELECT output_id, name, type, hash FROM outputs WHERE uuid=?");
    _queryAssetByUuidStmt = _db.prepare(
        "SELECT uuid, source_db_path, source_hash, asset_type, importer_name, importer_revision FROM assets WHERE "
        "uuid=?");
    _queryAssetBySourcePathStmt = _db.prepare(
        "SELECT uuid, source_db_path, source_hash, asset_type, importer_name, importer_revision FROM assets WHERE "
        "source_db_path=?");
    _insertAssetStmt = _db.prepare(
        "INSERT INTO assets "
        "(uuid, source_db_path, source_hash, asset_type, importer_name, importer_revision) "
        "VALUES(?, ?, ?, ?, ?, ?)"
        "ON CONFLICT (uuid) DO UPDATE SET source_hash=excluded.source_hash, asset_type=excluded.asset_type, "
        "importer_name=excluded.importer_name, importer_revision=excluded.importer_revision");
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
         _queryAssetsStmt.query<zstring_view, zstring_view, uint64, zstring_view, zstring_view, uint64>()) {
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
                createLogicalAssetId(UUID::fromString(uuid), logicalName),
                logicalName,
                outputType,
                outputHash,
                fullName);
        }
    }
}
