// Copyright (C) 2018 Sean u[_detail::d::index]dleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "math_traits.h"

namespace gm::swizzle::_detail {
    struct x {
        static constexpr int index = 0;
    };
    struct y {
        static constexpr int index = 1;
    };
    struct z {
        static constexpr int index = 2;
    };
    struct w {
        static constexpr int index = 3;
    };

    constexpr int required_vector_components(int a, int b, int c = 0, int d = 0) {
        int max = a;
        if (b > max)
            max = b;
        if (c > max)
            max = c;
        if (d > max)
            max = d;
        return max + 1;
    }
} // namespace gm::swizzle::_detail

#define _gm_MATH_SWIZZLE2(a, b) \
    template <typename T> \
    GM_MATHCALL a##b(T const& value)->std::enable_if_t<is_vector_v<T, _detail::required_vector_components(_detail::a::index, _detail::b::index)>, typename T::template vector_template<2>> { \
        return value.template shuffle<_detail::a::index, _detail::b::index>(); \
    }
#define _gm_MATH_SWIZZLE3(a, b, c) \
    template <typename T> \
    GM_MATHCALL a##b##c(T const& value)->std::enable_if_t<is_vector_v<T, _detail::required_vector_components(_detail::a::index, _detail::b::index, _detail::c::index)>, typename T::template vector_template<3>> { \
        return value.template shuffle<_detail::a::index, _detail::b::index, _detail::c::index>(); \
    }
#define _gm_MATH_SWIZZLE4(a, b, c, d) \
    template <typename T> \
    GM_MATHCALL a##b##c##d(T const& value)->std::enable_if_t<is_vector_v<T, _detail::required_vector_components(_detail::a::index, _detail::b::index, _detail::c::index, _detail::d::index)>, typename T::template vector_template<4>> { \
        return value.template shuffle<_detail::a::index, _detail::b::index, _detail::c::index, _detail::d::index>(); \
    }

namespace gm::swizzle {
    _gm_MATH_SWIZZLE2(x, y);
    _gm_MATH_SWIZZLE2(y, x);
    _gm_MATH_SWIZZLE2(y, z);
    _gm_MATH_SWIZZLE2(z, w);
    _gm_MATH_SWIZZLE2(w, z);

    _gm_MATH_SWIZZLE3(x, y, z);
    _gm_MATH_SWIZZLE3(z, y, x);
    _gm_MATH_SWIZZLE3(y, z, w);

    _gm_MATH_SWIZZLE4(x, y, z, w);
    _gm_MATH_SWIZZLE4(x, x, x, x);
    _gm_MATH_SWIZZLE4(w, z, y, x);
} // namespace gm::swizzle

#undef _gm_MATH_SWIZZLE2
#undef _gm_MATH_SWIZZLE3
#undef _gm_MATH_SWIZZLE4
