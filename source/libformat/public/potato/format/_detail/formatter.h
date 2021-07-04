// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"

namespace up {
    /// Generic formatter
    template <typename T>
    struct formatter {
        // constexpr char const* parse(string_view) noexcept;

        // template <typename OutputT>
        // constexpr void format(OutputT& out, T const& value);
    };

    /// Formatter for void types
    template <>
    struct formatter<void> {
        constexpr char const* parse(string_view spec) noexcept { return spec.end(); }

        template <typename OutputT, typename ValueT>
        constexpr void format(OutputT&, ValueT const&) {}
    };
} // namespace up
