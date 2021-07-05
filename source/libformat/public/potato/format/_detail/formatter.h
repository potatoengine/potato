// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/concepts.h"
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
        constexpr explicit format_parse_context(char const* begin, char const* end) noexcept
            : _begin(begin)
            , _end(end) {}

        constexpr char const* begin() const noexcept { return _begin; }
        constexpr char const* end() const noexcept { return _end; }

        constexpr void advance_to(char const* ptr) noexcept { _begin = ptr; }

    private:
        char const* _begin = nullptr;
        char const* _end = nullptr;
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
    concept formatter_for =
        requires(char* output, FormatterT& formatter, format_parse_context& ctx, ValueT const& value) {
        { formatter.parse(ctx) }
        ->convertible_to<decltype(ctx.begin())>;
        formatter.format(output, value);
    };

    /// Trait to determine if a formatter for a given value exist
    template <typename ValueT, typename FormatterT = formatter<remove_cvref_t<ValueT>>>
    constexpr bool has_formatter_v = formatter_for<FormatterT, ValueT>;
} // namespace up
