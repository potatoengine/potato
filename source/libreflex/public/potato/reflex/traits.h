// Copyright (C) 2019 Potato Engine authors and contributors, all rights reserverd.

#pragma once

namespace up {
    class string_view;
    class zstring_view;
    class string;
    template <typename T>
    class vector;
} // namespace up

namespace up::reflex {
    /// True if the provided type is a recognized string type
    template <typename T>
    constexpr bool is_string_v = std::is_same_v<T, up::string_view> || std::is_same_v<T, up::zstring_view> || std::is_same_v<T, up::string>;
    static_assert(is_string_v<string_view>);
    static_assert(!is_string_v<char*>);

    /// True if the provided type is an integral or floating point type
    template <typename T>
    constexpr bool is_numeric_v = std::is_integral_v<T> || std::is_floating_point_v<T>;
    static_assert(is_numeric_v<bool> && is_numeric_v<char> && is_numeric_v<float>);
    static_assert(!is_numeric_v<int*> && !is_numeric_v<float&> && !is_numeric_v<std::nullptr_t>);

    /// True if the provided type is a vector specialization
    namespace _detail {
        template <typename T>
        struct is_vector { constexpr static bool value = false; };
        template <typename T>
        struct is_vector<vector<T>> { constexpr static bool value = true; };
    }
    template <typename T>
    constexpr bool is_vector_v = _detail::is_vector<T>::value;
} // namespace up::reflex
