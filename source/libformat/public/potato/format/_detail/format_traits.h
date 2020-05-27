// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

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
    template <typename T> using decay_array_t = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T> const*, T>;

    template <typename T> using formattable_t = decay_array_t<std::remove_reference_t<T>>;

    template <typename W, typename T, typename V = void> struct has_format_value { static constexpr bool value = false; };
    template <typename W, typename T> struct has_format_value<W, T, std::void_t<decltype(format_value(std::declval<W&>(), std::declval<T>()))>> {
        static constexpr bool value = true;
    };

    template <typename T>
    constexpr bool is_native_formattable_v =
        std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> || std::is_convertible_v<T, char const*>;

    template <typename T> constexpr bool is_formattable_v = is_native_formattable_v<T> || has_format_value<_empty_format_writer, T>::value;
} // namespace up::_detail

namespace up {
    template <typename T> concept format_writable = requires(T& w) { {w.write(string_view{})}; };

    template <typename T> concept format_appendable = requires(T& w) { {w.append("", 0)}; };

    template <typename T> concept formattable = _detail::is_formattable_v<std::remove_cvref_t<T>>;
} // namespace up
