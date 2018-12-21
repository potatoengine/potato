// Copyright (C) 2017 Sean Middleditch, all rights reserverd.

#pragma once

#include <type_traits>

namespace gm {
    template <typename... T>
    struct typelist {};
    template <typename T>
    struct typelist_size;

    template <template <class...> class T, typename U>
    struct typelist_apply;
    template <template <class...> class T, typename... U>
    struct typelist_apply<T, typelist<U...>> { using type = T<U...>; };

    template <typename... T>
    struct typelist_size<typelist<T...>> { static constexpr size_t value = sizeof...(T); };
    template <typename T>
    constexpr size_t typelist_size_v = typelist_size<T>::value;
    template <template <class...> class T, typename U>
    using typelist_apply_t = typename typelist_apply<T, U>::type;
} // namespace gm
