// Copyright (C) 2017 Sean Middleditch, all rights reserverd.

#pragma once

#include <type_traits>

namespace gm {
    template <typename... T>
    struct typelist {};
} // namespace gm

namespace gm::_detail {
    template <typename U>
    struct typelist_head;
    template <typename T, typename... R>
    struct typelist_head<typelist<T, R...>> { using type = T; };

    template <typename U>
    struct typelist_tail;
    template <typename T, typename... R>
    struct typelist_tail<typelist<T, R...>> { using type = typelist<R...>; };

    template <template <class...> class T, typename U>
    struct typelist_apply;
    template <template <class...> class T, typename... U>
    struct typelist_apply<T, typelist<U...>> { using type = T<U...>; };

    template <template <class...> class T, typename U>
    struct typelist_map;
    template <template <class...> class T, typename... U>
    struct typelist_map<T, typelist<U...>> { using type = typelist<T<U>...>; };
} // namespace gm::_detail

namespace gm {
    template <typename T>
    using typelist_head_t = typename _detail::typelist_head<T>::type;

    static_assert(std::is_same_v<typelist_head_t<typelist<int, float, char>>, int>);

    template <typename T>
    using typelist_tail_t = typename _detail::typelist_tail<T>::type;

    static_assert(std::is_same_v<typelist_tail_t<typelist<int, float, char>>, typelist<float, char>>);

    template <typename T>
    constexpr std::size_t typelist_size_v = 0;
    template <typename... T>
    constexpr auto typelist_size_v<typelist<T...>> = sizeof...(T);

    static_assert(typelist_size_v<typelist<>> == 0);
    static_assert(typelist_size_v<typelist<int>> == 1);
    static_assert(typelist_size_v<typelist<int, float, char>> == 3);

    template <template <class...> class T, typename U>
    using typelist_apply_t = typename _detail::typelist_apply<T, U>::type;

    static_assert(typelist_apply_t<std::is_same, typelist<int, int>>::value);

    template <template <class...> class T, typename U>
    using typelist_map_t = typename _detail::typelist_map<T, U>::type;

    static_assert(std::is_same_v<typelist_map_t<std::add_const_t, typelist<int, char>>, typelist<int const, char const>>);
} // namespace gm
