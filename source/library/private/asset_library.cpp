// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "_export.h"
#include "grimm/library/asset_library.h"
#include "grimm/foundation/fnv1a.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include "stream_json.h"

static constexpr gm::uint64 libraryRevision = 3;

gm::AssetLibrary::~AssetLibrary() = default;

auto gm::AssetLibrary::pathToAssetId(string_view path) const -> AssetId {
    fnv1a hasher;
    hasher({reinterpret_cast<byte const*>(path.data()), path.size()});
    return static_cast<AssetId>(static_cast<uint64>(hasher));
}

auto gm::AssetLibrary::assetIdToPath(AssetId assetId) const -> string_view {
    auto record = findRecord(assetId);
    return record != nullptr ? string_view(record->path) : string_view{};
}

auto gm::AssetLibrary::findRecord(AssetId assetId) const -> AssetImportRecord const* {
    auto it = _assets.find(assetId);
    return it != _assets.end() ? &it->second : nullptr;
}

bool gm::AssetLibrary::insertRecord(AssetImportRecord record) {
    _assets[record.assetId] = std::move(record);
    return true;
}

bool gm::AssetLibrary::serialize(fs::Stream& stream) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto root = doc.GetObject();

    root.AddMember("revision", rapidjson::Value(libraryRevision), doc.GetAllocator());

    auto array = rapidjson::Value(rapidjson::kArrayType);
    for (auto const& [assetId, record] : _assets) {
        auto recObj = rapidjson::Value(rapidjson::kObjectType);
        recObj.AddMember("id", rapidjson::Value(static_cast<uint64>(record.assetId)), doc.GetAllocator());
        recObj.AddMember("path", rapidjson::Value(rapidjson::StringRef(record.path.data(), record.path.size())), doc.GetAllocator());
        recObj.AddMember("contentHash", rapidjson::Value(record.contentHash), doc.GetAllocator());
        auto catName = assetCategoryName(record.category);
        recObj.AddMember("category", rapidjson::Value(rapidjson::StringRef(catName.data(), catName.size())), doc.GetAllocator());
        recObj.AddMember("importerName", rapidjson::Value(rapidjson::StringRef(record.importerName.data(), record.importerName.size())), doc.GetAllocator());
        recObj.AddMember("importerRevision", rapidjson::Value(record.importerRevision), doc.GetAllocator());

        auto outputs = rapidjson::Value(rapidjson::kArrayType);
        for (auto const& output : record.outputs) {
            auto outputObj = rapidjson::Value(rapidjson::kObjectType);
            outputObj.AddMember("path", rapidjson::Value(rapidjson::StringRef(output.path.data(), output.path.size())), doc.GetAllocator());
            outputObj.AddMember("hash", rapidjson::Value(output.contentHash), doc.GetAllocator());
            outputs.PushBack(outputObj, doc.GetAllocator());
        }
        recObj.AddMember("outputs", outputs, doc.GetAllocator());

        auto sourceDeps = rapidjson::Value(rapidjson::kArrayType);
        for (auto const& dep : record.sourceDependencies) {
            auto depObj = rapidjson::Value(rapidjson::kObjectType);
            depObj.AddMember("path", rapidjson::Value(rapidjson::StringRef(dep.path.data(), dep.path.size())), doc.GetAllocator());
            depObj.AddMember("hash", rapidjson::Value(dep.contentHash), doc.GetAllocator());
            sourceDeps.PushBack(depObj, doc.GetAllocator());
        }
        recObj.AddMember("sourceDeps", sourceDeps, doc.GetAllocator());

        array.PushBack(recObj, doc.GetAllocator());
    }
    root.AddMember("records", array, doc.GetAllocator());

    RapidJsonStreamWrapper outWrapper(stream);
    rapidjson::Writer writer(outWrapper);
    return doc.Accept(writer);
}

bool gm::AssetLibrary::deserialize(fs::Stream& stream) {
    RapidJsonStreamWrapper inWrapper(stream);
    rapidjson::Document doc;
    doc.ParseStream(inWrapper);
    if (doc.HasParseError()) {
        return false;
    }

    if (!doc.IsObject()) {
        return false;
    }

    auto root = doc.GetObject();

    if (!root.HasMember("revision") || !root["revision"].IsUint64() || root["revision"].GetUint64() != libraryRevision) {
        return false;
    }

    if (!root.HasMember("records")) {
        return false;
    }

    auto& records = root[rapidjson::Value("records")];
    if (!records.IsArray()) {
        return false;
    }

    for (auto& record : records.GetArray()) {
        if (!record.HasMember("id") || !record["id"].IsUint64()) {
            continue;
        }
        if (!record.HasMember("path") || !record["path"].IsString()) {
            continue;
        }
        if (!record.HasMember("contentHash") || !record["contentHash"].IsUint64()) {
            continue;
        }
        if (!record.HasMember("category") || !record["category"].IsString()) {
            continue;
        }
        if (!record.HasMember("importerName") || !record["importerName"].IsString()) {
            continue;
        }
        if (!record.HasMember("importerRevision") || !record["importerRevision"].IsUint64()) {
            continue;
        }

        if (!record.HasMember("outputs") || !record["outputs"].IsArray()) {
            continue;
        }

        AssetImportRecord newRecord;
        newRecord.assetId = static_cast<AssetId>(record["id"].GetUint64());
        newRecord.path = string(record["path"].GetString());
        newRecord.contentHash = record["contentHash"].GetUint64();
        newRecord.category = assetCategoryFromName(record["category"].GetString());
        newRecord.importerName = string(record["importerName"].GetString());
        newRecord.importerRevision = record["importerRevision"].GetUint64();

        for (auto const& output : record["outputs"].GetArray()) {
            if (!output.IsObject()) {
                continue;
            }

            auto pathIt = output.FindMember("path");
            auto hashIt = output.FindMember("hash");

            if (pathIt == output.MemberEnd() || !pathIt->value.IsString()) {
                continue;
            }

            if (hashIt == output.MemberEnd() || !hashIt->value.IsUint64()) {
                continue;
            }

            newRecord.outputs.push_back(AssetOutputRecord{
                string(pathIt->value.GetString()),
                hashIt->value.GetUint64()});
        }

        for (auto const& output : record["sourceDeps"].GetArray()) {
            if (!output.IsObject()) {
                continue;
            }

            auto pathIt = output.FindMember("path");
            auto hashIt = output.FindMember("hash");

            if (pathIt == output.MemberEnd() || !pathIt->value.IsString()) {
                continue;
            }

            if (hashIt == output.MemberEnd() || !hashIt->value.IsUint64()) {
                continue;
            }

            newRecord.sourceDependencies.push_back(AssetDependencyRecord{
                string(pathIt->value.GetString()),
                hashIt->value.GetUint64()});
        }

        insertRecord(std::move(newRecord));
    }

    return true;
}
