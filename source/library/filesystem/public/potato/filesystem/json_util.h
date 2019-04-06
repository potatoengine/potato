// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <nlohmann/json_fwd.hpp>

namespace up {
    class string_view;
    class string;

    extern void UP_FILESYSTEM_API to_json(nlohmann::json& json, string_view str) noexcept;
    extern void UP_FILESYSTEM_API from_json(const nlohmann::json& json, string& str) noexcept;
} // namespace up
