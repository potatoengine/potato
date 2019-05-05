// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/json_util.h"
#include "potato/filesystem/stream.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include <nlohmann/json.hpp>

auto up::readJson(Stream& stream, nlohmann::json& json) -> IOResult {
    string text;
    IOResult rs = readText(stream, text);
    if (rs != IOResult::Success) {
        return rs;
    }

    json = nlohmann::json::parse(text.begin(), text.end(), nullptr, false);
    if (!json.is_object()) {
        return IOResult::Malformed;
    }

    return IOResult::Success;
}

void up::to_json(nlohmann::json& json, string_view str) noexcept {
    json = std::string(str.data(), str.size());
}

void up::from_json(const nlohmann::json& json, string& str) noexcept {
    str.reset();
    if (json.is_string()) {
        const std::string& stdString = json;
        str = string(stdString.data(), stdString.size());
    }
}
