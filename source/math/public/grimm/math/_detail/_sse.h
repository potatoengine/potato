// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"

#if !defined(GM_ARCH_INTEL)
#    error "SSE can only be used on Intel architectures"
#endif

#include <cmath>
#include <cstdint>

#include <pmmintrin.h>
#include <xmmintrin.h>

#define _gm_SSE_MATHCALL GM_FORCEINLINE auto GM_VECTORCALL

// SSE vector modeled on: http://www.codersnotes.com/notes/maths-lib-2016/

namespace gm {
    class Vector4f {
    public:
        static constexpr int component_length = 4;

        using value_type = float;
        using const_array_type = value_type const[component_length];

        GM_FORCEINLINE Vector4f() : v(_mm_set1_ps(0.f)) {}
        explicit GM_FORCEINLINE Vector4f(const value_type* p) : v(_mm_set_ps(p[2], p[2], p[1], p[0])) {}
        explicit GM_FORCEINLINE Vector4f(value_type x, value_type y, value_type z, value_type w) : v(_mm_set_ps(w, z, y, x)) {}
        explicit GM_FORCEINLINE Vector4f(vec_broadcast_t, value_type value) : v(_mm_set1_ps(value)) {}
        /*implicit*/ GM_FORCEINLINE Vector4f(__m128 vec) : v(vec) {}

        _gm_SSE_MATHCALL x() const->value_type { return _mm_cvtss_f32(v); }
        _gm_SSE_MATHCALL y() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1))); }
        _gm_SSE_MATHCALL z() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))); }
        _gm_SSE_MATHCALL w() const->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))); }

        _gm_SSE_MATHCALL wzyx() const->Vector4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 1, 2, 3)); }

        friend _gm_SSE_MATHCALL operator+(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_add_ps(lhs.v, rhs.v); }
        friend _gm_SSE_MATHCALL operator-(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_sub_ps(lhs.v, rhs.v); }
        friend _gm_SSE_MATHCALL operator*(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_mul_ps(lhs.v, rhs.v); }
        friend _gm_SSE_MATHCALL operator/(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_div_ps(lhs.v, rhs.v); }

        friend _gm_SSE_MATHCALL operator+(Vector4f lhs, value_type rhs)->Vector4f { return _mm_add_ps(lhs.v, _mm_set1_ps(rhs)); }
        friend _gm_SSE_MATHCALL operator-(Vector4f lhs, value_type rhs)->Vector4f { return _mm_sub_ps(lhs.v, _mm_set1_ps(rhs)); }
        friend _gm_SSE_MATHCALL operator*(Vector4f lhs, value_type rhs)->Vector4f { return _mm_mul_ps(lhs.v, _mm_set1_ps(rhs)); }
        friend _gm_SSE_MATHCALL operator/(Vector4f lhs, value_type rhs)->Vector4f { return _mm_div_ps(lhs.v, _mm_set1_ps(rhs)); }

        friend _gm_SSE_MATHCALL operator+(value_type lhs, Vector4f rhs)->Vector4f { return _mm_add_ps(_mm_set1_ps(lhs), rhs.v); }
        friend _gm_SSE_MATHCALL operator-(value_type lhs, Vector4f rhs)->Vector4f { return _mm_sub_ps(_mm_set1_ps(lhs), rhs.v); }
        friend _gm_SSE_MATHCALL operator*(value_type lhs, Vector4f rhs)->Vector4f { return _mm_mul_ps(_mm_set1_ps(lhs), rhs.v); }
        friend _gm_SSE_MATHCALL operator/(value_type lhs, Vector4f rhs)->Vector4f { return _mm_div_ps(_mm_set1_ps(lhs), rhs.v); }

        friend _gm_SSE_MATHCALL operator-(Vector4f vec)->Vector4f { return _mm_sub_ps(_mm_setzero_ps(), vec.v); }

        friend _gm_SSE_MATHCALL operator==(Vector4f lhs, Vector4f rhs)->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) == 15;
        }
        friend _gm_SSE_MATHCALL operator!=(Vector4f lhs, Vector4f rhs)->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) != 15;
        }

        _gm_SSE_MATHCALL operator[](int index) const->value_type {
            alignas(16) value_type a[component_length];
            _mm_store_ps(a, v);
            return a[index];
        }

        friend _gm_SSE_MATHCALL min(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_min_ps(lhs.v, rhs.v); }
        friend _gm_SSE_MATHCALL max(Vector4f lhs, Vector4f rhs)->Vector4f { return _mm_max_ps(lhs.v, rhs.v); }

        friend _gm_SSE_MATHCALL abs(Vector4f vec)->Vector4f { return _mm_andnot_ps({0x80000000, 0x80000000, 0x80000000, 0x80000000}, vec.v); }

        friend _gm_SSE_MATHCALL clamp(Vector4f t, Vector4f a, Vector4f b)->Vector4f { return min(max(t, a), b); }
        friend _gm_SSE_MATHCALL horizontalAdd(Vector4f vec)->value_type {
            __m128 t = _mm_hadd_ps(vec.v, vec.v);
            return _mm_cvtss_f32(_mm_hadd_ps(t, t));
        }
        friend _gm_SSE_MATHCALL dot(Vector4f lhs, Vector4f rhs)->value_type { return horizontalAdd(lhs * rhs); }
        friend _gm_SSE_MATHCALL squareLength(Vector4f vec)->value_type { return dot(vec, vec); }
        friend _gm_SSE_MATHCALL length(Vector4f vec)->value_type { return std::sqrt(dot(vec, vec)); }
        friend _gm_SSE_MATHCALL reciprocal(Vector4f vec)->Vector4f { return _mm_rcp_ps(vec.v); }
        friend _gm_SSE_MATHCALL normalize(Vector4f vec)->Vector4f { return vec * reciprocal(Vector4f{vec_broadcast, length(vec)}); }
        friend _gm_SSE_MATHCALL lerp(Vector4f a, Vector4f b, value_type t)->Vector4f { return a + (b - a) * t; }

    public:
        __m128 v;
    };

    using Vector = Vector4f;

    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
    static_assert(alignof(Vector4f) == 16);
} // namespace gm

#undef _gm_SSE_MATHCALL
