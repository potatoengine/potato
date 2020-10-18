// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_library.h"

#include "potato/runtime/json.h"
#include "potato/runtime/resource_manifest.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/out_ptr.h"
#include "potato/spud/string_writer.h"

#include <sqlite3.h>

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
    if (_db == nullptr)
        return false;

    // create our prepared statemtnt to save values
    unique_resource<sqlite3_stmt*, sqlite3_finalize> assets_stmt;
    unique_resource<sqlite3_stmt*, sqlite3_finalize> outputs_stmt;
    unique_resource<sqlite3_stmt*, sqlite3_finalize> dependencies_stmt;

    sqlite3_prepare_v3(
        _db.get(),
        "INSERT INTO assets "
        "(asset_id, source_db_path, source_hash, importer_name, importer_revision) "
        "VALUES(?, ?, ?, ?, ?)",
        -1,
        0,
        out_ptr(assets_stmt),
        nullptr);
    sqlite3_prepare_v3(
        _db.get(),
        "INSERT INTO outputs (asset_id, output_id, name, hash) VALUES(?, ?, ?, ?)",
        -1,
        0,
        out_ptr(outputs_stmt),
        nullptr);
    sqlite3_prepare_v3(
        _db.get(),
        "INSERT INTO dependencies (asset_id, db_path, hash) VALUES(?, ?, ?)",
        -1,
        0,
        out_ptr(dependencies_stmt),
        nullptr);

    for (auto const& record : _records) {
        sqlite3_reset(assets_stmt.get());
        sqlite3_bind_int64(assets_stmt.get(), 1, to_underlying(record.assetId));
        sqlite3_bind_text(assets_stmt.get(), 2, record.sourcePath.c_str(), -1, nullptr);
        sqlite3_bind_int64(assets_stmt.get(), 3, record.sourceContentHash);
        sqlite3_bind_text(assets_stmt.get(), 4, record.importerName.c_str(), -1, nullptr);
        sqlite3_bind_int64(assets_stmt.get(), 5, record.importerRevision);
        sqlite3_step(assets_stmt.get());

        for (auto const& output : record.outputs) {
            sqlite3_reset(outputs_stmt.get());
            sqlite3_bind_int64(outputs_stmt.get(), 1, to_underlying(record.assetId));
            sqlite3_bind_int64(outputs_stmt.get(), 2, to_underlying(output.logicalAssetId));
            sqlite3_bind_text(outputs_stmt.get(), 3, output.name.c_str(), -1, nullptr);
            sqlite3_bind_int64(outputs_stmt.get(), 4, output.contentHash);
            sqlite3_step(outputs_stmt.get());
        }

        for (auto const& dep : record.dependencies) {
            sqlite3_reset(dependencies_stmt.get());
            sqlite3_bind_int64(dependencies_stmt.get(), 1, to_underlying(record.assetId));
            sqlite3_bind_text(dependencies_stmt.get(), 2, dep.path.c_str(), -1, nullptr);
            sqlite3_bind_int64(dependencies_stmt.get(), 3, dep.contentHash);
            sqlite3_step(dependencies_stmt.get());
        }
    }

    return true;
}

bool up::AssetLibrary::loadDatabase(zstring_view filename) {
    // open the database
    {
        _db.reset();
        auto const rs = sqlite3_open(filename.c_str(), out_ptr(_db));
        if (rs != SQLITE_OK)
            return false;
    }

    // ensure the table is created
    {
        auto const rs = sqlite3_exec(
            _db.get(),
            "CREATE TABLE IF NOT EXISTS assets "
            "(asset_id INTEGER PRIMARY KEY, "
            "source_db_path TEXT, source_hash INTEGER, "
            "importer_name TEXT, importer_revision INTEGER);\n"

            "CREATE TABLE IF NOT EXISTS outputs "
            "(asset_id INTEGER, output_id INTEGER, name TEXT, hash TEXT, "
            "FOREIGN KEY(asset_id) REFERENCES assets(asset_id));\n"

            "CREATE TABLE IF NOT EXISTS dependencies "
            "(asset_id INTEGER, db_path TEXT, hash TEXT, "
            "FOREIGN KEY(asset_id) REFERENCES assets(asset_id));\n",
            nullptr,
            nullptr,
            nullptr);
        if (rs != SQLITE_OK) {
            _db.reset();
            return false;
        }
    }

    // create our prepared statemtnt to load values
    unique_resource<sqlite3_stmt*, sqlite3_finalize> assets_stmt;
    unique_resource<sqlite3_stmt*, sqlite3_finalize> outputs_stmt;
    unique_resource<sqlite3_stmt*, sqlite3_finalize> dependencies_stmt;

    sqlite3_prepare_v3(
        _db.get(),
        "SELECT asset_id, source_db_path, source_hash, importer_name, importer_revision FROM assets",
        -1,
        0,
        out_ptr(assets_stmt),
        nullptr);
    sqlite3_prepare_v3(
        _db.get(),
        "SELECT output_id, name, hash FROM outputs WHERE asset_id=?",
        -1,
        0,
        out_ptr(outputs_stmt),
        nullptr);
    sqlite3_prepare_v3(
        _db.get(),
        "SELECT db_path, hash FROM dependencies WHERE asset_id=?",
        -1,
        0,
        out_ptr(dependencies_stmt),
        nullptr);

    // read in all the asset records
    while (sqlite3_step(assets_stmt.get()) == SQLITE_ROW) {
        auto& record = _records.push_back({});
        record.assetId = static_cast<AssetId>(sqlite3_column_int64(assets_stmt.get(), 0));

        record.sourcePath = reinterpret_cast<char const*>(sqlite3_column_text(assets_stmt.get(), 1));
        record.sourceContentHash = sqlite3_column_int64(assets_stmt.get(), 2);

        record.importerName = reinterpret_cast<char const*>(sqlite3_column_text(assets_stmt.get(), 3));
        record.importerRevision = sqlite3_column_int64(assets_stmt.get(), 4);

        sqlite3_reset(outputs_stmt.get());
        sqlite3_bind_int64(outputs_stmt.get(), 1, static_cast<int64>(record.assetId));
        while (sqlite3_step(outputs_stmt.get()) == SQLITE_ROW) {
            auto& output = record.outputs.emplace_back();
            output.logicalAssetId = static_cast<AssetId>(sqlite3_column_int64(outputs_stmt.get(), 0));
            output.name = reinterpret_cast<char const*>(sqlite3_column_text(outputs_stmt.get(), 1));
            output.contentHash = sqlite3_column_int64(outputs_stmt.get(), 2);
        }

        sqlite3_reset(dependencies_stmt.get());
        sqlite3_bind_int64(dependencies_stmt.get(), 1, static_cast<int64>(record.assetId));
        while (sqlite3_step(dependencies_stmt.get()) == SQLITE_ROW) {
            auto& dependency = record.dependencies.emplace_back();
            dependency.path = reinterpret_cast<char const*>(sqlite3_column_text(dependencies_stmt.get(), 0));
            dependency.contentHash = sqlite3_column_int64(dependencies_stmt.get(), 1);
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
