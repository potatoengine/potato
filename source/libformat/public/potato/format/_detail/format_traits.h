// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <potato/spud/string_view.h>
#include <potato/spud/traits.h>

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

    template <typename T, typename V = void>
    struct _is_format_writer_helper { static constexpr bool value = false; };
    template <typename T>
    struct _is_format_writer_helper<T, std::void_t<decltype(std::declval<T>().write(std::declval<string_view>()))>> {
        static constexpr bool value = true;
    };

    template <typename W, typename T, typename V = void>
    struct _has_format_value { static constexpr bool value = false; };
    template <typename W, typename T>
    struct _has_format_value<W, T, std::void_t<decltype(format_value(std::declval<W&>(), std::declval<T>()))>> {
        static constexpr bool value = true;
    };

    template <typename T>
    constexpr bool _is_string = std::is_convertible_v<T, char const*>;

    template <typename T>
    constexpr bool _is_formattable = std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> || ::up::_detail::_is_string<T> || _has_format_value<::up::_empty_format_writer, T>::value;
} // namespace up::_detail

namespace up {
    template <typename T>
    constexpr bool is_format_writer = ::up::_detail::_is_format_writer_helper<T>::value;

    template <typename Writer, typename T>
    constexpr bool has_format_value = ::up::_detail::_has_format_value<Writer, T>::value;

    template <typename T>
    constexpr bool is_formattable = ::up::_detail::_is_formattable<::up::remove_cvref_t<T>>;
} // namespace up
