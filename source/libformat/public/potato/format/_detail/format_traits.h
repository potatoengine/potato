// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include <potato/spud/string_view.h>
#include <type_traits>

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

    template <typename T>
    constexpr bool is_format_writer = _is_format_writer_helper<T>::value;

} // up::_detail
