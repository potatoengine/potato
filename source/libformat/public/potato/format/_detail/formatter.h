// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

namespace up {
    /// Generic formatter
    template <typename T>
    struct formatter {
        // constexpr char const* parse(format_parse_context& ctx) noexcept;

        // template <typename OutputT>
        // constexpr void format(OutputT& out, T const& value);
    };

    /// Formatter parse context
    class format_parse_context {
    public:
        constexpr explicit format_parse_context(string_view spec) noexcept : _spec(spec) {}

        constexpr char const* begin() const noexcept { return _spec.begin(); }
        constexpr char const* end() const noexcept { return _spec.end(); }

    private:
        string_view _spec;
    };

    /// Formatter for void types
    template <>
    struct formatter<void> {
        constexpr char const* parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

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
