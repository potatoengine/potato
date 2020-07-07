// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "common.h"

#include <nlohmann/json_fwd.hpp>

namespace up {
    class string_view;
    class zstring_view;
    class string;
    class Stream;

    extern UP_RUNTIME_API auto readJson(Stream& stream, nlohmann::json& json) -> IOResult;

    // nlohmann overloads
    extern UP_RUNTIME_API void to_json(nlohmann::json& json, string_view str) noexcept;
    extern UP_RUNTIME_API void to_json(nlohmann::json& json, zstring_view str) noexcept;
    extern UP_RUNTIME_API void to_json(nlohmann::json& json, string const& str) noexcept;
    extern UP_RUNTIME_API void from_json(const nlohmann::json& json, string& str);
} // namespace up
