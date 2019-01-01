// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

#define _gm_MATH_SWIZZLE2(a, b, N) \
    template <typename T, typename = std::enable_if_t<is_vector_v<T, N>>> \
    GM_MATHCALL a##b(T const& value)->typename T::template vector_template<2> { \
        return {value.m.a, value.m.b}; \
    }
#define _gm_MATH_SWIZZLE3(a, b, c, N) \
    template <typename T, typename = std::enable_if_t<is_vector_v<T, N>>> \
    GM_MATHCALL a##b##c(T const& value)->typename T::template vector_template<3> { \
        return {value.m.a, value.m.b, value.m.c}; \
    }
#define _gm_MATH_SWIZZLE4(a, b, c, d, N) \
    template <typename T, typename = std::enable_if_t<is_vector_v<T, N>>> \
    GM_MATHCALL a##b##c##d(T const& value)->typename T::template vector_template<4> { \
        return {value.m.a, value.m.b, value.m.c, value.m.d}; \
    }

namespace gm::swizzle {
    _gm_MATH_SWIZZLE2(x, y, 2);
    _gm_MATH_SWIZZLE2(y, x, 2);
    _gm_MATH_SWIZZLE2(y, z, 3);
    _gm_MATH_SWIZZLE2(z, w, 4);
    _gm_MATH_SWIZZLE2(w, z, 4);

    _gm_MATH_SWIZZLE3(x, y, z, 3);
    _gm_MATH_SWIZZLE3(z, y, x, 3);
    _gm_MATH_SWIZZLE3(y, z, w, 4);

    _gm_MATH_SWIZZLE4(x, y, z, w, 4);
    _gm_MATH_SWIZZLE4(w, z, y, x, 4);
} // namespace gm::swizzle

#undef _gm_MATH_SWIZZLE2
