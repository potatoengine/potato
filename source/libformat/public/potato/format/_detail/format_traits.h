// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "formatter.h"

#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

namespace up {
    /// @internal
    // Has to be in up namespace due to some unresolved ADL shenanigans, for now
    struct _empty_format_writer {
        void write(string_view);
    };
} // namespace up

namespace up::_detail {
    template <typename T>
    using decay_array_t = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T> const*, T>;

    template <typename T>
    using formattable_t = decay_array_t<std::remove_reference_t<T>>;

    template <typename OutputT, typename ValueT>
    concept has_format_value = requires(OutputT& output, formatter<ValueT>& formatter, ValueT const& value) {
        formatter.format(output, value);
    };

    template <typename FormatterT>
    concept has_formatter_parse = requires(FormatterT& formatter) {
        formatter.parse(string_view{});
    };

    template <typename T>
    constexpr bool is_native_formattable_v =
        std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> ||
        std::is_convertible_v<T, char const*> || std::is_convertible_v<T, string_view>;

    template <typename T>
    constexpr bool is_formattable_v = is_native_formattable_v<T> || has_format_value<_empty_format_writer, T>;
} // namespace up::_detail

namespace up {
    template <typename T>
    concept format_writable = requires(T& w) {
        {w.write(string_view{})};
    };

    template <typename T>
    concept format_appendable = requires(T& w) {
        {w.append("", 0)};
    };

    template <typename T>
    concept formattable = _detail::is_formattable_v<std::remove_cvref_t<T>>;
} // namespace up
