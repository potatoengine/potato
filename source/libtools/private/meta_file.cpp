// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "meta_file.h"

#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>

void up::MetaFile::generate() { uuid = UUID::generate(); }

auto up::MetaFile::toJson() const -> string {
    nlohmann::json doc;

    doc["id"] = uuid.toString();
    auto importerJson = nlohmann::json();
    importerJson["name"] = importerName;
    importerJson["settings"] = importerSettings;
    doc["importer"] = std::move(importerJson);

    return string(doc.dump(2).c_str());
}

bool up::MetaFile::parseJson(string_view json) {
    auto const doc = nlohmann::json::parse(json);

    if (auto const idJson = doc["id"]; idJson.is_string()) {
        uuid = UUID::fromString(idJson.get<string>());
    }

    if (auto const importerJson = doc["importer"]; importerJson.is_object()) {
        if (auto const importerNameJson = importerJson["name"]; importerNameJson.is_string()) {
            importerName = importerNameJson.get<string>();
        }
        if (auto const importerSettingsJson = importerJson["settings"]; importerSettingsJson.is_string()) {
            importerSettings = importerSettingsJson.get<string>();
        }
    }

    return uuid.isValid();
}
