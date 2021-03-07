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
    for (auto const& [uuid] : _db.query<UUID>("SELECT uuid FROM assets WHERE source_path=?", path)) {
        return uuid;
    }

    return {};
}

auto up::AssetDatabase::uuidToPath(UUID const& uuid) noexcept -> string {
    for (auto const& [sourcePath] : _db.query<zstring_view>("SELECT source_path FROM assets WHERE uuid=?", uuid)) {
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
    for (auto const& [sourcePath] : _db.query<zstring_view>("SELECT source_path FROM "
                                                            "assets")) {
        co_yield sourcePath;
    }
}

auto up::AssetDatabase::collectAssetsDirtiedBy(zstring_view dependencyPath, uint64 dependencyHash)
    -> generator<zstring_view const> {
    for (auto const& [sourcePath] : _db.query<zstring_view>(
             "SELECT source_path FROM assets INNER JOIN import_dependencies ON assets.uuid=import_dependencies.uuid "
             "WHERE "
             "import_dependencies.path=? AND import_dependencies.hash<>?",
             dependencyPath,
             dependencyHash)) {
        co_yield sourcePath;
    }
}

auto up::AssetDatabase::assetDependencies(UUID const& uuid) -> generator<Dependency const> {
    for (auto const& [path, hash] :
         _db.query<zstring_view, uint64>("SELECT path, hash FROM import_dependencies WHERE uuid=?", uuid)) {
        co_yield Dependency{.path = string{path}, .contentHash = hash};
    }
}

auto up::AssetDatabase::assetOutputs(UUID const& uuid) -> generator<Output const> {
    for (auto const& [id, name, type, hash] : _db.query<AssetId, zstring_view, zstring_view, uint64>(
             "SELECT output_id, name, type, hash FROM outputs WHERE uuid=?",
             uuid)) {
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
    [[maybe_unused]] auto const rc = _db.execute(
        "INSERT INTO assets "
        "(uuid, source_path, source_hash, status) VALUES(?, ?, ?, 'NEW')"
        "ON CONFLICT (uuid) DO UPDATE SET source_path=excluded.source_path, "
        "source_hash=excluded.source_hash",
        uuid,
        sourcePath,
        sourceHash);
    UP_ASSERT(rc == SqlResult::Ok);
}

bool up::AssetDatabase::checkAssetUpToDate(
    UUID const& uuid,
    string_view importerName,
    uint64 importerVersion,
    uint64 sourceHash) {
    auto const [upToDate] = _db.queryOne<bool>(
        "SELECT (source_hash=? AND importer_name=? AND importer_revision=? AND status='IMPORTED') AS "
        "up_to_date FROM "
        "assets WHERE uuid=?",
        sourceHash,
        importerName,
        importerVersion,
        uuid);
    return upToDate;
}

void up::AssetDatabase::updateAssetPre(
    UUID const& uuid,
    string_view importerName,
    string_view assetType,
    uint64 importerVersion) {
    [[maybe_unused]] auto const rc = _db.execute(
        "UPDATE assets SET importer_name=?, asset_type=?, importer_revision=?, status='PENDING' WHERE "
        "uuid=?",
        importerName,
        assetType,
        importerVersion,
        uuid);
    UP_ASSERT(rc == SqlResult::Ok);
}

void up::AssetDatabase::updateAssetPost(UUID const& uuid, bool success) {
    [[maybe_unused]] auto rc =
        _db.execute("UPDATE assets SET status=? WHERE uuid=?", success ? "IMPORTED"_sv : "FAILED"_sv, uuid);
    UP_ASSERT(rc == SqlResult::Ok);
    (void)_db.execute("DELETE FROM compiled WHERE uuid=?", uuid);
    (void)_db.execute("DELETE FROM import_dependencies WHERE uuid=?", uuid);
}

bool up::AssetDatabase::deleteAsset(UUID const& uuid) {
    (void)_db.execute("DELETE FROM assets WHERE uuid=?", uuid);
    return true;
}

void up::AssetDatabase::addDependency(UUID const& uuid, zstring_view outputPath, uint64 outputHash) {
    (void)
        _db.execute("INSERT INTO import_dependencies (uuid, path, hash) VALUES(?, ?, ?)", uuid, outputPath, outputHash);
}

void up::AssetDatabase::addOutput(UUID const& uuid, zstring_view name, zstring_view assetType, uint64 outputHash) {
    (void)_db.execute(
        "INSERT INTO compiled (id, uuid, name, type, hash) VALUES(?, ?, ?, ?, ?)",
        createLogicalAssetId(uuid, name),
        uuid,
        name,
        assetType,
        outputHash);
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
            (void)_db.execute("DROP TABLE IF EXISTS assets");
            (void)_db.execute("DROP TABLE IF EXISTS dependencies");
            (void)_db.execute("DROP TABLE IF EXISTS outputs");
            (void)_db.execute("DROP TABLE IF EXISTS import_dependencies");
            (void)_db.execute("DROP TABLE IF EXISTS compiled");
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
                    "importer_name TEXT, importer_revision INTEGER)") != SqlResult::Ok) {
        return false;
    }
    if (_db.execute("CREATE TABLE IF NOT EXISTS compiled "
                    "(id INT NOT NULL PRIMARY KEY, uuid TEXT, name TEXT, type TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid))") != SqlResult::Ok) {
        return false;
    }
    if (_db.execute("CREATE TABLE IF NOT EXISTS import_dependencies "
                    "(uuid TEXT, path TEXT, hash TEXT, "
                    "FOREIGN KEY(uuid) REFERENCES assets(uuid), "
                    "UNIQUE(uuid, path))") != SqlResult::Ok) {
        return false;
    }

    // ensure any left-over assets are transitioned to failed status
    (void)_db.execute("UPDATE assets SET status='FAILED' WHERE status<>'IMPORTED'");

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

    for (auto const& [uuid, sourcePath, assetType] :
         _db.query<zstring_view, zstring_view, zstring_view>("SELECT uuid, source_path, asset_type FROM "
                                                             "assets")) {
        format_to(writer, "{}|||{}||{}\n", uuid, assetType, sourcePath);
    }

    for (auto const& [uuid, sourcePath, assetType] :
         _db.query<UUID, zstring_view, zstring_view>("SELECT uuid, source_path, asset_type FROM "
                                                     "assets")) {
        for (auto const& [logicalId, logicalName, outputType, outputHash] :
             _db.query<AssetId, zstring_view, zstring_view, uint64>(
                 "SELECT id, name, type, hash FROM compiled WHERE uuid=?",
                 uuid)) {
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
