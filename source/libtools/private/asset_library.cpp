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
    for (auto& current : _records) {
        if (current.assetId == record.assetId) {
            current = std::move(record);
            return true;
        }
    }

    _records.push_back(std::move(record));
    return true;
}

bool up::AssetLibrary::saveDatabase() {
    // create our prepared statemtnt to save values
    auto assets_stmt = _db.prepare(
        "INSERT INTO assets "
        "(asset_id, source_db_path, source_hash, importer_name, importer_revision) "
        "VALUES(?, ?, ?, ?, ?)");
    auto outputs_stmt = _db.prepare("INSERT INTO outputs (asset_id, output_id, name, hash) VALUES(?, ?, ?, ?)");
    auto dependencies_stmt = _db.prepare("INSERT INTO dependencies (asset_id, db_path, hash) VALUES(?, ?, ?)");

    for (auto const& record : _records) {
        assets_stmt.bind(0, to_underlying(record.assetId));
        assets_stmt.bind(1, record.sourcePath.c_str());
        assets_stmt.bind(2, record.sourceContentHash);
        assets_stmt.bind(3, record.importerName.c_str());
        assets_stmt.bind(4, record.importerRevision);
        (void)assets_stmt.execute();

        for (auto const& output : record.outputs) {
            outputs_stmt.bind(0, to_underlying(record.assetId));
            outputs_stmt.bind(1, to_underlying(output.logicalAssetId));
            outputs_stmt.bind(2, output.name.c_str());
            outputs_stmt.bind(3, output.contentHash);
            (void)outputs_stmt.execute();
        }

        for (auto const& dep : record.dependencies) {
            outputs_stmt.bind(0, to_underlying(record.assetId));
            outputs_stmt.bind(1, dep.path.c_str());
            outputs_stmt.bind(2, dep.contentHash);
            (void)outputs_stmt.execute();
        }
    }

    return true;
}

bool up::AssetLibrary::loadDatabase(zstring_view filename) {
    // open the database
    if (_db.open(filename.c_str()) != SqlResult::Ok)
        return false;

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

    // create our prepared statemtnt to load values
    auto assets_stmt =
        _db.prepare("SELECT asset_id, source_db_path, source_hash, importer_name, importer_revision FROM assets");
    auto outputs_stmt = _db.prepare("SELECT output_id, name, hash FROM outputs WHERE asset_id=?");
    auto dependencies_stmt = _db.prepare("SELECT db_path, hash FROM dependencies WHERE asset_id=?");

    // read in all the asset records
    for (auto row : assets_stmt.query()) {
        auto& record = _records.push_back({});
        record.assetId = static_cast<AssetId>(row.get_int64(0));

        record.sourcePath = row.get_string(1);
        record.sourceContentHash = row.get_int64(2);

        record.importerName = row.get_string(3);
        record.importerRevision = row.get_int64(4);

        outputs_stmt.bind(0, static_cast<int64>(record.assetId));
        for (auto output_row : outputs_stmt.query()) {
            auto& output = record.outputs.emplace_back();
            output.logicalAssetId = static_cast<AssetId>(output_row.get_int64(0));
            output.name = output_row.get_string(1);
            output.contentHash = output_row.get_int64(2);
        }

        dependencies_stmt.bind(0, static_cast<int64>(record.assetId));
        for (auto dep_row : dependencies_stmt.query()) {
            auto& dependency = record.dependencies.emplace_back();
            dependency.path = dep_row.get_string(0);
            dependency.contentHash = dep_row.get_int64(1);
        }
    }

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
