// Copyright (C) 2018 Sean u[_detail::d::index]dleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/const_util.h"
#include "grimm/foundation/preprocessor.h"
#include <type_traits>

namespace gm::swizzle::_detail::component {
    static constexpr int x = 0;
    static constexpr int y = 1;
    static constexpr int z = 2;
    static constexpr int w = 3;
} // namespace gm::swizzle::_detail::component

#define _gm_MATH_SWIZZLE_INDEX(xxx) ::gm::swizzle::_detail::component::xxx
#define _gm_MATH_SWIZZLE_INDICES(...) GM_PP_MAP(_gm_MATH_SWIZZLE_INDEX, __VA_ARGS__)
#define _gm_MATH_SWIZZLE_MIN_LENGTH(...) (::gm::max_v<_gm_MATH_SWIZZLE_INDICES(__VA_ARGS__)> + 1)

#define GM_DEFINE_SWIZZLE(...) \
    template <typename T> \
    GM_MATHCALL GM_PP_JOIN(__VA_ARGS__)(T value) noexcept \
        ->::gm::enable_if_t< \
            T::component_length >= _gm_MATH_SWIZZLE_MIN_LENGTH(__VA_ARGS__), \
            typename T::template vector_template<GM_PP_ARITY(__VA_ARGS__)>> { \
        return value.template shuffle<_gm_MATH_SWIZZLE_INDICES(__VA_ARGS__)>(); \
    }

namespace gm::swizzle {
    GM_DEFINE_SWIZZLE(x, y);
    GM_DEFINE_SWIZZLE(y, x);
    GM_DEFINE_SWIZZLE(y, z);
    GM_DEFINE_SWIZZLE(z, w);
    GM_DEFINE_SWIZZLE(w, z);

    GM_DEFINE_SWIZZLE(x, y, z);
    GM_DEFINE_SWIZZLE(z, y, x);
    GM_DEFINE_SWIZZLE(y, z, w);

    GM_DEFINE_SWIZZLE(x, y, z, w);
    GM_DEFINE_SWIZZLE(x, x, x, x);
    GM_DEFINE_SWIZZLE(w, z, y, x);
} // namespace gm::swizzle
