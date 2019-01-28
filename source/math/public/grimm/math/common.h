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
} // namespace gm
