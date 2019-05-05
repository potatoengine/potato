// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/assetdb/_export.h"
#include "potato/assetdb/asset_library.h"
#include "potato/foundation/hash_fnv1a.h"
#include "potato/foundation/hash.h"
#include "potato/filesystem/stream.h"
#include "potato/filesystem/json_util.h"
#include <nlohmann/json.hpp>

static constexpr up::uint64 libraryRevision = 3;

up::AssetLibrary::~AssetLibrary() = default;

auto up::AssetLibrary::pathToAssetId(string_view path) const -> AssetId {
    auto hash = hash_value<fnv1a>(path);
    return static_cast<AssetId>(hash);
}

auto up::AssetLibrary::assetIdToPath(AssetId assetId) const -> string_view {
    auto record = findRecord(assetId);
    return record != nullptr ? string_view(record->path) : string_view{};
}

auto up::AssetLibrary::findRecord(AssetId assetId) const -> AssetImportRecord const* {
    auto it = _assets.find(assetId);
    return it != _assets.end() ? &it->second : nullptr;
}

bool up::AssetLibrary::insertRecord(AssetImportRecord record) {
    _assets[record.assetId] = std::move(record);
    return true;
}

bool up::AssetLibrary::serialize(Stream& stream) const {
    nlohmann::json jsonRoot;

    jsonRoot["revision"] = libraryRevision;

    nlohmann::json jsonRecords;
    for (auto const& [assetId, record] : _assets) {
        nlohmann::json jsonRecord;

        auto catName = assetCategoryName(record.category);
        
        jsonRecord["id"] = static_cast<uint64>(record.assetId);
        jsonRecord["path"] = std::string(record.path.data(), record.path.size());
        jsonRecord["contentHash"] = record.contentHash;
        jsonRecord["category"] = std::string(catName.data(), catName.size());
        jsonRecord["importerName"] = std::string(record.importerName.data(), record.importerName.size());
        jsonRecord["importerRevision"] = record.importerRevision;

        nlohmann::json jsonOutputs;
        for (auto const& output : record.outputs) {
            nlohmann::json jsonOutput;
            jsonOutput["path"] = std::string(output.path.data(), output.path.size());
            jsonOutput["hash"] = output.contentHash;
            jsonOutputs.push_back(std::move(jsonOutput));
        }
        jsonRecord["outputs"] = std::move(jsonOutputs);

        nlohmann::json jsonSourceDeps;
        for (auto const& dep : record.sourceDependencies) {
            nlohmann::json jsonDep;
            jsonDep["path"] = std::string(dep.path.data(), dep.path.size());
            jsonDep["hash"] = dep.contentHash;
            jsonSourceDeps.push_back(std::move(jsonDep));
        }
        jsonRecord["sourceDeps"] = std::move(jsonSourceDeps);

        jsonRecords.push_back(std::move(jsonRecord));
    }
    jsonRoot["records"] = std::move(jsonRecords);

    auto json = jsonRoot.dump(2);
    return writeAllText(stream, {json.data(), json.size()}) == IOResult::Success;
}

bool up::AssetLibrary::deserialize(Stream& stream) {
    string jsonText;
    if (readText(stream, jsonText) != IOResult::Success) {
        return false;
    }

    auto jsonRoot = nlohmann::json::parse(jsonText, nullptr, false);

    if (jsonRoot.is_discarded()) {
        return false;
    }

    auto revision = jsonRoot["revision"];
    if (!revision.is_number_integer() || revision != libraryRevision) {
        return false;
    }

    auto records = jsonRoot["records"];
    if (!records.is_array()) {
        return false;
    }

    for (auto const& record : records) {
        AssetImportRecord newRecord;
        newRecord.assetId = record["id"];
        newRecord.path = record["path"].get<string>();
        newRecord.contentHash = record["contentHash"];
        newRecord.category = assetCategoryFromName(record["category"].get<string>());
        newRecord.importerName = record["importerName"].get<string>();
        newRecord.importerRevision = record["importerRevision"];

        for (auto const& output : record["outputs"]) {
            newRecord.outputs.push_back(AssetOutputRecord{
                output["path"],
                output["hash"]
            });
        }

        for (auto const& output : record["sourceDeps"]) {
            newRecord.sourceDependencies.push_back(AssetDependencyRecord{
                output["path"],
                output["hash"]
            });
        }

        insertRecord(std::move(newRecord));
    }

    return true;
}
