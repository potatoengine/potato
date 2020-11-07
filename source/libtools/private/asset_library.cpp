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

auto up::AssetLibrary::pathToAssetId(string_view path) const -> AssetId {
    auto hash = hash_value<fnv1a>(path);
    return static_cast<AssetId>(hash);
}

auto up::AssetLibrary::assetIdToPath(AssetId assetId) const -> string_view {
    auto record = findRecord(assetId);
    return record != nullptr ? string_view(record->sourcePath) : string_view{};
}

auto up::AssetLibrary::findRecord(AssetId assetId) const -> Imported const* {
    for (auto const& record : _records) {
        if (record.assetId == assetId) {
            return &record;
        }
    }
    return nullptr;
}

bool up::AssetLibrary::insertRecord(Imported record) {
    // update database
    if (_insertAssetStmt) {
        auto tx = _db.begin();
        (void)_insertAssetStmt.execute(
            record.assetId,
            record.sourcePath.c_str(),
            record.sourceContentHash,
            record.importerName.c_str(),
            record.importerRevision);

        (void)_clearOutputsStmt.execute(record.assetId);
        for (auto const& output : record.outputs) {
            (void)_insertOutputStmt
                .execute(record.assetId, output.logicalAssetId, output.name.c_str(), output.contentHash);
        }

        (void)_clearDependenciesStmt.execute(record.assetId);
        for (auto const& dep : record.dependencies) {
            (void)_insertDependencyStmt.execute(record.assetId, dep.path.c_str(), dep.contentHash);
        }

        tx.commit();
    }

    // update in-memory data
    for (auto& current : _records) {
        if (current.assetId == record.assetId) {
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

    // ensure the table is created
    if (_db.execute("CREATE TABLE IF NOT EXISTS assets "
                    "(asset_id INTEGER PRIMARY KEY, "
                    "source_db_path TEXT, source_hash INTEGER, "
                    "importer_name TEXT, importer_revision INTEGER);\n"

                    "CREATE TABLE IF NOT EXISTS outputs "
                    "(asset_id INTEGER, output_id INTEGER, name TEXT, hash TEXT, "
                    "FOREIGN KEY(asset_id) REFERENCES assets(asset_id));\n"

                    "CREATE TABLE IF NOT EXISTS dependencies "
                    "(asset_id INTEGER, db_path TEXT, hash TEXT, "
                    "FOREIGN KEY(asset_id) REFERENCES assets(asset_id));\n") != SqlResult::Ok) {
        return false;
    }

    // create our prepared statements for later use
    _insertAssetStmt = _db.prepare(
        "INSERT INTO assets "
        "(asset_id, source_db_path, source_hash, importer_name, importer_revision) "
        "VALUES(?, ?, ?, ?, ?)"
        "ON CONFLICT (asset_id) DO UPDATE SET source_hash=excluded.source_hash, "
        " importer_name=excluded.importer_name, importer_revision=excluded.importer_revision");
    _insertOutputStmt = _db.prepare("INSERT INTO outputs (asset_id, output_id, name, hash) VALUES(?, ?, ?, ?)");
    _insertDependencyStmt = _db.prepare("INSERT INTO dependencies (asset_id, db_path, hash) VALUES(?, ?, ?)");
    _clearOutputsStmt = _db.prepare("DELETE FROM outputs WHERE asset_id=?");
    _clearDependenciesStmt = _db.prepare("DELETE FROM dependencies WHERE asset_id=?");

    // create our prepared statemtnt to load values
    auto assets_stmt =
        _db.prepare("SELECT asset_id, source_db_path, source_hash, importer_name, importer_revision FROM assets");
    auto outputs_stmt = _db.prepare("SELECT output_id, name, hash FROM outputs WHERE asset_id=?");
    auto dependencies_stmt = _db.prepare("SELECT db_path, hash FROM dependencies WHERE asset_id=?");

    // read in all the asset records
    for (auto const& [assetId, southPath, sourceHash, importName, importVer] :
         assets_stmt.query<AssetId, zstring_view, uint64, zstring_view, uint64>()) {
        auto& record = _records.push_back(Imported{
            .assetId = assetId,
            .sourcePath = string{southPath},
            .importerName = string{importName},
            .importerRevision = importVer,
            .sourceContentHash = sourceHash});

        for (auto const& [id, name, hash] : outputs_stmt.query<AssetId, zstring_view, uint64>(assetId)) {
            record.outputs.push_back(Output{.name = string{name}, .logicalAssetId = id, .contentHash = hash});
        }

        for (auto const& [path, hash] : dependencies_stmt.query<zstring_view, uint64>(assetId)) {
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
        ResourceManifest::columnRootId,
        ResourceManifest::columnLogicalId,
        ResourceManifest::columnLogicalName,
        ResourceManifest::columnContentHash,
        ResourceManifest::columnDebugName);

    string_writer fullName;

    for (auto const& record : _records) {
        for (auto const& output : record.outputs) {
            fullName.clear();
            fullName.append(record.sourcePath);

            if (output.logicalAssetId != record.assetId) {
                format_append(fullName, ":{}", output.name);
            }

            format_to(
                writer,
                "{:016X}|{:016X}|{:016X}|{:016X}|{}\n",
                record.assetId,
                output.logicalAssetId,
                hash_value(output.name),
                output.contentHash,
                fullName);
        }
    }
}
