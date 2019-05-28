// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "common.h"
#include <nlohmann/json_fwd.hpp>

namespace up {
    class string_view;
    class string;
    class Stream;

    extern auto UP_FILESYSTEM_API readJson(Stream& stream, nlohmann::json& json) -> IOResult;

    // nlohmann overloads
    extern void UP_FILESYSTEM_API to_json(nlohmann::json& json, string_view str) noexcept;
    extern void UP_FILESYSTEM_API from_json(const nlohmann::json& json, string& str) noexcept;
} // namespace up
