// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_library.h"

#include "potato/runtime/json.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/string_writer.h"

up::AssetLibrary::~AssetLibrary() = default;

auto up::AssetLibrary::pathToUuid(string_view path) const noexcept -> UUID {
    for (auto const& record : _records) {
        if (record.sourcePath == path) {
            return record.uuid;
        }
    }
    return {};
}

auto up::AssetLibrary::uuidToPath(UUID const& uuid) const noexcept -> string_view {
    auto record = findRecordByUuid(uuid);
    return record != nullptr ? string_view(record->sourcePath) : string_view{};
}

auto up::AssetLibrary::findRecordByUuid(UUID const& uuid) const noexcept -> Imported const* {
    for (auto const& record : _records) {
        if (record.uuid == uuid) {
            return &record;
        }
    }
    return nullptr;
}

auto up::AssetLibrary::createLogicalAssetId(UUID const& uuid, string_view logicalName) noexcept -> AssetId {
    uint64 hash = hash_value(uuid);
    if (!logicalName.empty()) {
        hash = hash_combine(hash, hash_value(logicalName));
    }
    return static_cast<AssetId>(hash);
}

bool up::AssetLibrary::insertRecord(Imported record) {
    char uuidStr[UUID::strLength] = {};
    format_to(uuidStr, "{}", record.uuid);

    // update database
    if (_insertAssetStmt) {
        auto tx = _db.begin();
        (void)_insertAssetStmt.execute(
            uuidStr,
            record.sourcePath.c_str(),
            record.sourceContentHash,
            record.importerName.c_str(),
            record.importerRevision);

        (void)_clearOutputsStmt.execute(uuidStr);
        for (auto const& output : record.outputs) {
            (void)_insertOutputStmt
                .execute(uuidStr, output.logicalAssetId, output.name.c_str(), output.type.c_str(), output.contentHash);
        }

        (void)_clearDependenciesStmt.execute(uuidStr);
        for (auto const& dep : record.dependencies) {
            (void)_insertDependencyStmt.execute(uuidStr, dep.path.c_str(), dep.contentHash);
        }

        tx.commit();
    }

    // update in-memory data
    for (auto& current : _records) {
        if (current.uuid == record.uuid) {
            current = std::move(record);
            return true;
        }
    }

    _records.push_back(std::move(record));
    return true;
}

bool up::AssetLibrary::open(zstring_view filename) {
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
                    "source_db_path TEXT, source_hash INTEGER, "
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
    _insertAssetStmt = _db.prepare(
        "INSERT INTO assets "
        "(uuid, source_db_path, source_hash, importer_name, importer_revision) "
        "VALUES(?, ?, ?, ?, ?)"
        "ON CONFLICT (uuid) DO UPDATE SET source_hash=excluded.source_hash, "
        " importer_name=excluded.importer_name, importer_revision=excluded.importer_revision");
    _insertOutputStmt = _db.prepare("INSERT INTO outputs (uuid, output_id, name, type, hash) VALUES(?, ?, ?, ?, ?)");
    _insertDependencyStmt = _db.prepare("INSERT INTO dependencies (uuid, db_path, hash) VALUES(?, ?, ?)");
    _clearOutputsStmt = _db.prepare("DELETE FROM outputs WHERE uuid=?");
    _clearDependenciesStmt = _db.prepare("DELETE FROM dependencies WHERE uuid=?");

    // create our prepared statemtnt to load values
    auto assets_stmt =
        _db.prepare("SELECT uuid, source_db_path, source_hash, importer_name, importer_revision FROM assets");
    auto outputs_stmt = _db.prepare("SELECT output_id, name, type, hash FROM outputs WHERE uuid=?");
    auto dependencies_stmt = _db.prepare("SELECT db_path, hash FROM dependencies WHERE uuid=?");

    // read in all the asset records
    for (auto const& [uuid, southPath, sourceHash, importName, importVer] :
         assets_stmt.query<zstring_view, zstring_view, uint64, zstring_view, uint64>()) {
        auto& record = _records.push_back(Imported{
            .uuid = UUID::fromString(uuid),
            .sourcePath = string{southPath},
            .importerName = string{importName},
            .importerRevision = importVer,
            .sourceContentHash = sourceHash});

        for (auto const& [id, name, type, hash] :
             outputs_stmt.query<AssetId, zstring_view, zstring_view, uint64>(uuid)) {
            record.outputs.push_back(
                Output{.name = string{name}, .type = string{type}, .logicalAssetId = id, .contentHash = hash});
        }

        for (auto const& [path, hash] : dependencies_stmt.query<zstring_view, uint64>(uuid)) {
            record.dependencies.push_back(Dependency{.path = string{path}, .contentHash = hash});
        }
    }

    return true;
}

bool up::AssetLibrary::close() {
    _db.close();
    return true;
}

void up::AssetLibrary::generateManifest(erased_writer writer) const {
    format_to(writer, "# Potato Manifest\n");
    format_to(writer, ".version={}\n", ResourceManifest::version);
    format_to(
        writer,
        ":{}|{}|{}|{}|{}\n",
        ResourceManifest::columnUuid,
        ResourceManifest::columnLogicalId,
        ResourceManifest::columnContentType,
        ResourceManifest::columnContentHash,
        ResourceManifest::columnDebugName);

    string_writer fullName;

    for (auto const& record : _records) {
        for (auto const& output : record.outputs) {
            fullName.clear();
            fullName.append(record.sourcePath);

            if (!output.name.empty()) {
                format_append(fullName, ":{}", output.name);
            }

            format_to(
                writer,
                "{}|{:016X}|{}|{:016X}|{}\n",
                record.uuid,
                output.logicalAssetId,
                output.type,
                output.contentHash,
                fullName);
        }
    }
}
