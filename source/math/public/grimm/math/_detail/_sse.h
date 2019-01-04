// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"
#include "grimm/math/common.h"

#if !defined(GM_ARCH_INTEL)
#    error "SSE can only be used on Intel architectures"
#endif

#include <cmath>
#include <cstdint>

#include <pmmintrin.h>
#include <xmmintrin.h>

#define _gm_SSE_MATHCALL GM_FORCEINLINE auto GM_VECTORCALL
#define _gm_SSE_MATHCALL_FRIEND GM_FORCEINLINE friend auto GM_VECTORCALL
#if GM_COMPILER_MICROSOFT
#    define _gm_SSE_MATHCONSTRUCTOR GM_FORCEINLINE
#else
#    define _gm_SSE_MATHCONSTRUCTOR
#endif

// SSE vector modeled on: http://www.codersnotes.com/notes/maths-lib-2016/

namespace gm {
    class Vector4f {
    public:
        static constexpr int component_length = 4;

        using value_type = float;
        using const_array_type = value_type const[component_length];

        template <int N>
        using vector_template = Vector4f;

        _gm_SSE_MATHCONSTRUCTOR Vector4f() : v(_mm_set1_ps(0.f)) {}
        explicit _gm_SSE_MATHCONSTRUCTOR Vector4f(const value_type* p) : v(_mm_set_ps(p[2], p[2], p[1], p[0])) {}
        explicit _gm_SSE_MATHCONSTRUCTOR Vector4f(value_type x, value_type y, value_type z, value_type w) : v(_mm_set_ps(w, z, y, x)) {}
        explicit _gm_SSE_MATHCONSTRUCTOR Vector4f(vec_broadcast_t, value_type value) : v(_mm_set1_ps(value)) {}
        /*implicit*/ Vector4f(__m128 vec) : v(vec) {}

        template <int X = 0, int Y = 1, int Z = 2, int W = 3>
        _gm_SSE_MATHCALL shuffle() const { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W, Z, Y, X)); }

        _gm_SSE_MATHCALL x() const->value_type { return _mm_cvtss_f32(v); }
        _gm_SSE_MATHCALL y() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1))); }
        _gm_SSE_MATHCALL z() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))); }
        _gm_SSE_MATHCALL w() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))); }

        _gm_SSE_MATHCALL wzyx() const->Vector4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 1, 2, 3)); }

        _gm_SSE_MATHCALL_FRIEND operator+(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_add_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_sub_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator*(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_mul_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator/(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_div_ps(lhs.v, rhs.v); }

        _gm_SSE_MATHCALL_FRIEND operator+(Vector4f lhs, value_type rhs)->Vector4f { return _mm_add_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f lhs, value_type rhs)->Vector4f { return _mm_sub_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator*(Vector4f lhs, value_type rhs)->Vector4f { return _mm_mul_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator/(Vector4f lhs, value_type rhs)->Vector4f { return _mm_div_ps(lhs.v, _mm_set1_ps(rhs)); }

        _gm_SSE_MATHCALL_FRIEND operator+(value_type lhs, Vector4f rhs)->Vector4f { return _mm_add_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator-(value_type lhs, Vector4f rhs)->Vector4f { return _mm_sub_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator*(value_type lhs, Vector4f rhs)->Vector4f { return _mm_mul_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator/(value_type lhs, Vector4f rhs)->Vector4f { return _mm_div_ps(_mm_set1_ps(lhs), rhs.v); }

        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f vec)->Vector4f { return _mm_sub_ps(_mm_setzero_ps(), vec.v); }

        _gm_SSE_MATHCALL_FRIEND operator==(Vector4f lhs, Vector4f rhs)->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) == 15;
        }
        _gm_SSE_MATHCALL_FRIEND operator!=(Vector4f lhs, Vector4f rhs)->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) != 15;
        }

        _gm_SSE_MATHCALL operator[](int index) const->value_type {
            alignas(16) value_type a[component_length];
            _mm_store_ps(a, v);
            return a[index];
        }

        _gm_SSE_MATHCALL_FRIEND min(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_min_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND max(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_max_ps(lhs.v, rhs.v); }

        _gm_SSE_MATHCALL_FRIEND abs(Vector4f vec)->Vector4f {
            auto signBits = _mm_set_ps(0x80000000, 0x80000000, 0x80000000, 0x80000000);
            return _mm_andnot_ps(signBits, vec.v);
        }

        _gm_SSE_MATHCALL_FRIEND clamp(Vector4f t, Vector4f a, Vector4f b)->Vector4f { return min(max(t, a), b); }
        _gm_SSE_MATHCALL_FRIEND horizontalAdd(Vector4f vec)->value_type {
            __m128 t = _mm_hadd_ps(vec.v, vec.v);
            return _mm_cvtss_f32(_mm_hadd_ps(t, t));
        }
        _gm_SSE_MATHCALL_FRIEND dot(Vector4f lhs, Vector4f rhs)->value_type { return horizontalAdd(lhs * rhs); }
        _gm_SSE_MATHCALL_FRIEND squareLength(Vector4f vec)->value_type { return dot(vec, vec); }
        _gm_SSE_MATHCALL_FRIEND length(Vector4f vec)->value_type { return std::sqrt(dot(vec, vec)); }
        _gm_SSE_MATHCALL_FRIEND reciprocal(Vector4f vec)->Vector4f { return _mm_rcp_ps(vec.v); }
        _gm_SSE_MATHCALL_FRIEND normalize(Vector4f vec)->Vector4f { return vec * reciprocal(Vector4f{vec_broadcast, length(vec)}); }
        _gm_SSE_MATHCALL_FRIEND lerp(Vector4f a, Vector4f b, value_type t)->Vector4f { return a + (b - a) * t; }

    public:
        __m128 v;
    };

    using Vector = Vector4f;

    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
    static_assert(alignof(Vector4f) == 16);
} // namespace gm

#undef _gm_SSE_MATHCALL
