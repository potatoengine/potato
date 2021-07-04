// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "formatter.h"

#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

namespace up::_detail {
    template <typename T>
    using decay_array_t = std::conditional_t<std::is_array_v<T>, std::remove_extent_t<T> const*, T>;

    template <typename T>
    using formattable_t = decay_array_t<std::remove_reference_t<T>>;

    template <typename T>
    constexpr bool is_native_formattable_v =
        std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T> || std::is_enum_v<T> ||
        std::is_convertible_v<T, char const*> || std::is_convertible_v<T, string_view>;

    template <typename T>
    constexpr bool is_formattable_v = is_native_formattable_v<T> || has_formatter_v<T>;
} // namespace up::_detail

namespace up {
    template <typename T>
    concept formattable = _detail::is_formattable_v<std::remove_cvref_t<T>>;
} // namespace up
