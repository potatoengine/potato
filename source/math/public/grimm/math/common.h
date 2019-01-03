// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"
#include <type_traits>

#define GM_MATHCALL_CONVERSION GM_FORCEINLINE constexpr GM_VECTORCALL
#define GM_MATHCALL_FRIEND GM_FORCEINLINE friend constexpr auto GM_VECTORCALL
#define GM_MATHCALL GM_FORCEINLINE constexpr auto GM_VECTORCALL

namespace gm {
    static constexpr struct {
    } vec_broadcast;
    using vec_broadcast_t = decltype(vec_broadcast);

    template <typename T, int MinN>
    constexpr bool is_vector_v = (std::is_same_v<void, std::void_t<typename T::value_type>> && (T::component_length >= MinN));
} // namespace gm
