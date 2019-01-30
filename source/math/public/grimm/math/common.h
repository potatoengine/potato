// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"

#define GM_MATHCALL_CONVERSION GM_FORCEINLINE constexpr GM_VECTORCALL
#define GM_MATHCALL_FRIEND GM_FORCEINLINE friend constexpr auto GM_VECTORCALL
#define GM_MATHCALL GM_FORCEINLINE constexpr auto GM_VECTORCALL

namespace gm {
    static constexpr struct {
    } vec_broadcast;
    using vec_broadcast_t = decltype(vec_broadcast);

    static constexpr struct {
    } noinit;
    using noinit_t = decltype(noinit);

    template <typename T>
    struct vector_size {
        static constexpr bool is_vector = false;
    };

    template <typename T>
    constexpr bool is_vector_v = vector_size<T>::is_vector;

    template <typename T>
    constexpr int component_length_v = vector_size<T>::size;

    template <typename T, int N>
    using vector_resized_t = typename vector_size<T>::template resize_t<N>;
} // namespace gm
