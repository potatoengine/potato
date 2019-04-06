// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/filesystem/json_util.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include <nlohmann/json.hpp>

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
