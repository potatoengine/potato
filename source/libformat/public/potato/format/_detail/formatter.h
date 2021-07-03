// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_result.h"

#include "potato/spud/string_view.h"

namespace up {
    /// @brief Generic formatter
    template <typename T>
    struct formatter {
        // constexpr format_result parse(string_view) noexcept;

        // template <typename OutputT>
        // constexpr void format(OutputT& out, T const& value);
    };

    /// @brief Formatter for void types
    template <>
    struct formatter<void> {
        constexpr format_result parse(string_view) noexcept { return format_result::success; }

        template <typename OutputT, typename ValueT>
        constexpr void format(OutputT&, ValueT const&) {}
    };
} // namespace up
