// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/library/hash_cache.h"
#include "grimm/foundation/fnv1a.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>

auto gm::HashCache::hashAssetContent(span<gm::byte const> contents) noexcept -> gm::uint64 {
    auto hasher = fnv1a();
    hasher(contents);
    return static_cast<uint64>(hasher);
}

auto gm::HashCache::hashAssetStream(std::istream& stream) -> gm::uint64 {
    auto hasher = fnv1a();
    char buffer[32768];
    while (stream) {
        stream.read(buffer, sizeof(buffer));
        auto count = stream.gcount();
        hasher(span(reinterpret_cast<byte const*>(buffer), count));
    }
    return static_cast<uint64>(hasher);
}

auto gm::HashCache::hashAssetAtPath(zstring_view path) -> gm::uint64 {
    fs::FileStat stat;
    auto rs = _fileSystem.fileStat(path, stat);
    if (rs != fs::Result::Success) {
        return 0;
    }

    auto it = _hashes.find(path);
    if (it != _hashes.end()) {
        if (rs == fs::Result::Success && stat.size == it->second->size && stat.mtime == it->second->mtime) {
            return it->second->hash;
        }
    }

    auto fstream = _fileSystem.openRead(path);
    auto hash = hashAssetStream(fstream);

    // update the hash
    auto rec = make_box<HashRecord>();
    rec->osPath = string(path);
    rec->hash = hash;
    rec->mtime = stat.mtime;
    rec->size = stat.size;
    _hashes[rec->osPath] = std::move(rec);
    return hash;
}

bool gm::HashCache::serialize(std::ostream& stream) const {
    rapidjson::Document doc;
    doc.SetObject();
    auto root = doc.GetObject();

    for (auto const& [key, value] : _hashes) {
        auto obj = rapidjson::Value(rapidjson::kObjectType);
        obj.AddMember("hash", rapidjson::Value(value->hash), doc.GetAllocator());
        obj.AddMember("mtime", rapidjson::Value(value->mtime), doc.GetAllocator());
        obj.AddMember("size", rapidjson::Value(value->size), doc.GetAllocator());
        root.AddMember(rapidjson::StringRef(key.data(), key.size()), obj, doc.GetAllocator());
    }

    rapidjson::OStreamWrapper outWrapper(stream);
    rapidjson::Writer writer(outWrapper);
    return doc.Accept(writer);
}

bool gm::HashCache::deserialize(std::istream& stream) {
    rapidjson::IStreamWrapper inWrapper(stream);
    rapidjson::Document doc;
    doc.ParseStream(inWrapper);
    if (doc.HasParseError()) {
        return false;
    }

    if (!doc.IsObject()) {
        return false;
    }

    for (auto const& member : doc.GetObject()) {
        if (!member.value.IsObject()) {
            continue;
        }

        auto hashIt = member.value.FindMember("hash");
        if (hashIt == member.value.MemberEnd() || !hashIt->value.IsUint64()) {
            continue;
        }

        auto mtimeIt = member.value.FindMember("mtime");
        if (mtimeIt == member.value.MemberEnd() || !mtimeIt->value.IsUint64()) {
            continue;
        }

        auto sizeIt = member.value.FindMember("size");
        if (sizeIt == member.value.MemberEnd() || !sizeIt->value.IsUint64()) {
            continue;
        }

        uint64 hash = hashIt->value.GetUint64();
        uint64 mtime = mtimeIt->value.GetUint64();
        uint64 size = sizeIt->value.GetUint64();

        string path(member.name.GetString());

        auto rec = make_box<HashRecord>();
        rec->osPath = string(path);
        rec->hash = hash;
        rec->mtime = mtime;
        rec->size = size;
        _hashes[rec->osPath] = std::move(rec);
        return hash;
    }

    return true;
}
