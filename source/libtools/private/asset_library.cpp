// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "asset_library.h"

#include "potato/runtime/json.h"
#include "potato/runtime/stream.h"
#include "potato/spud/hash.h"
#include "potato/spud/hash_fnv1a.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>

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

bool up::AssetLibrary::serialize(Stream& stream) const {
    nlohmann::json jsonRoot;

    jsonRoot["$type"] = typeName;
    jsonRoot["$version"] = version;

    nlohmann::json jsonRecords;
    for (auto const& record : _records) {
        nlohmann::json jsonRecord;

        jsonRecord["id"] = to_underlying(record.assetId);

        nlohmann::json jsonSource;
        jsonSource["path"] = std::string(record.sourcePath.data(), record.sourcePath.size());
        jsonSource["hash"] = record.sourceContentHash;
        jsonRecord["source"] = jsonSource;

        nlohmann::json jsonImporter;
        jsonImporter["name"] = std::string(record.importerName.data(), record.importerName.size());
        jsonImporter["revision"] = record.importerRevision;
        jsonRecord["importer"] = jsonImporter;

        nlohmann::json jsonOutputs;
        for (auto const& output : record.outputs) {
            nlohmann::json jsonOutput;
            jsonOutput["id"] = to_underlying(output.logicalAssetId);
            jsonOutput["hash"] = output.contentHash;
            jsonOutput["name"] = output.name;
            jsonOutputs.push_back(std::move(jsonOutput));
        }
        jsonRecord["outputs"] = std::move(jsonOutputs);

        nlohmann::json jsonDependencies;
        for (auto const& dep : record.dependencies) {
            nlohmann::json jsonDep;
            jsonDep["path"] = std::string(dep.path.data(), dep.path.size());
            jsonDep["hash"] = dep.contentHash;
            jsonDependencies.push_back(std::move(jsonDep));
        }
        jsonRecord["dependencies"] = std::move(jsonDependencies);

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

    if (auto type = jsonRoot["$type"]; !type.is_string() || type != typeName) {
        return false;
    }

    if (auto revision = jsonRoot["$version"]; !revision.is_number_integer() || revision != version) {
        return false;
    }

    auto records = jsonRoot["records"];
    if (!records.is_array()) {
        return false;
    }

    for (auto const& record : records) {
        auto& newRecord = _records.push_back({});

        newRecord.assetId = record["id"];

        if (auto const jsonSource = record["source"]; jsonSource.is_object()) {
            newRecord.sourcePath = jsonSource["path"].get<string>();
            newRecord.sourceContentHash = jsonSource["hash"];
        }

        if (auto const jsonImporter = record["importer"]; jsonImporter.is_object()) {
            newRecord.importerName = jsonImporter["name"].get<string>();
            newRecord.importerRevision = jsonImporter["revision"];
        }

        for (auto const& output : record["outputs"]) {
            newRecord.outputs.push_back({output["name"], output["id"], output["hash"]});
        }

        for (auto const& output : record["dependencies"]) {
            newRecord.dependencies.push_back({output["path"], output["hash"]});
        }
    }

    return true;
}

void up::AssetLibrary::generateManifest(erased_writer writer) const {
    format_to(writer,
        "# Potato Manifest\n"
        ".version={}\n"
        ":ID|HASH|NAME\n",
        version);

    string_writer logicalName;

    for (auto const& record : _records) {
        for (auto const& output : record.outputs) {
            logicalName.clear();
            logicalName.append(record.sourcePath);

            if (output.logicalAssetId != record.assetId) {
                format_append(logicalName, ":{}", output.name);
            }

            format_to(writer, "{:016X}|{:016X}|{}\n", output.logicalAssetId, output.contentHash, logicalName);
        }
    }
}
