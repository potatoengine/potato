// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_context.h"

#include "potato/spud/concepts.h"
#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

namespace up {
    /// Generic formatter
    template <typename T>
    struct formatter {
        // constexpr char const* parse(format_parse_context& ctx) noexcept;
        // constexpr void format(T const& value, format_context& ctx);
    };

    /// Formatter for void types
    template <>
    struct formatter<void> {
        constexpr char const* parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

        template <typename ValueT>
        constexpr void format(ValueT const&, format_context&) {}
    };
} // namespace up
