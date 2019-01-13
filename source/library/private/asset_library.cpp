// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "_export.h"
#include "grimm/library/asset_library.h"
#include "grimm/foundation/fnv1a.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/ostreamwrapper.h>

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

auto gm::AssetLibrary::findRecord(AssetId assetId) const -> AssetRecord const* {
    auto it = _assets.find(assetId);
    return it != _assets.end() ? &it->second : nullptr;
}

bool gm::AssetLibrary::insertRecord(AssetRecord record) {
    _assets[record.assetId] = std::move(record);
    return true;
}

bool gm::AssetLibrary::serialize(std::ostream& stream) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto root = doc.GetObject();

    auto array = rapidjson::Value(rapidjson::kArrayType);
    for (auto const& [assetId, record] : _assets) {
        auto recObj = rapidjson::Value(rapidjson::kObjectType);
        recObj.AddMember("id", rapidjson::Value(static_cast<uint64>(record.assetId)), doc.GetAllocator());
        recObj.AddMember("path", rapidjson::Value(rapidjson::StringRef(record.path.data(), record.path.size())), doc.GetAllocator());
        recObj.AddMember("contentHash", rapidjson::Value(record.contentHash), doc.GetAllocator());
        array.PushBack(recObj, doc.GetAllocator());
    }
    root.AddMember("records", array, doc.GetAllocator());

    rapidjson::OStreamWrapper outWrapper(stream);
    rapidjson::Writer writer(outWrapper);
    return doc.Accept(writer);
}

bool gm::AssetLibrary::deserialize(std::istream& stream) const {
    return false;
}
