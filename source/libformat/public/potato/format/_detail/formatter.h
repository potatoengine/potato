// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

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

    /// Concept to determine if a formatter accepts a given value type
    template <typename FormatterT, typename ValueT>
    concept formatter_for = requires(char* output, FormatterT& formatter, ValueT const& value) {
        formatter.format(output, value);
    };

    /// Trait to determine if a formatter for a given value exist
    template <typename ValueT, typename FormatterT = formatter<remove_cvref_t<ValueT>>>
    constexpr bool has_formatter_v = formatter_for<FormatterT, ValueT>;
} // namespace up
