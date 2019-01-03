// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include <grimm/foundation/platform.h>

#if !defined(GM_ARCH_INTEL)
#    error "SSE can only be used on Intel architectures"
#endif

#include <cmath>
#include <cstdint>

#include <xmmintrin.h>

#define _gm_SSE_MATHCALL GM_FORCEINLINE auto GM_VECTORCALL

namespace gm {
    class Vector4f {
    public:
        static constexpr int length = 4;

        using value_type = float;
        using const_array_type = value_type const[length];

        GM_FORCEINLINE Vector4f() : v(_mm_set1_ps(0.f)) {}
        explicit GM_FORCEINLINE Vector4f(const value_type* p) : v(_mm_set_ps(p[2], p[2], p[1], p[0])) {}
        explicit GM_FORCEINLINE Vector4f(value_type x, value_type y, value_type z, value_type w) : v(_mm_set_ps(w, z, y, x)) {}
        explicit GM_FORCEINLINE Vector4f(__m128 vec) : v(vec) {}

        _gm_SSE_MATHCALL x() const { return _mm_cvtss_f32(v); }
        _gm_SSE_MATHCALL y() const { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1))); }
        _gm_SSE_MATHCALL z() const { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))); }
        _gm_SSE_MATHCALL w() const { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))); }

        _gm_SSE_MATHCALL wzyx() const { return Vector4f{_mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 1, 2, 3))}; }

        friend _gm_SSE_MATHCALL operator+(Vector4f lhs, Vector4f rhs) { return Vector4f{_mm_add_ps(lhs.v, rhs.v)}; }
        friend _gm_SSE_MATHCALL operator-(Vector4f lhs, Vector4f rhs) { return Vector4f{_mm_sub_ps(lhs.v, rhs.v)}; }
        friend _gm_SSE_MATHCALL operator*(Vector4f lhs, Vector4f rhs) { return Vector4f{_mm_mul_ps(lhs.v, rhs.v)}; }
        friend _gm_SSE_MATHCALL operator/(Vector4f lhs, Vector4f rhs) { return Vector4f{_mm_div_ps(lhs.v, rhs.v)}; }

        friend _gm_SSE_MATHCALL operator+(Vector4f lhs, value_type rhs) { return Vector4f{_mm_add_ps(lhs.v, _mm_set1_ps(rhs))}; }
        friend _gm_SSE_MATHCALL operator-(Vector4f lhs, value_type rhs) { return Vector4f{_mm_sub_ps(lhs.v, _mm_set1_ps(rhs))}; }
        friend _gm_SSE_MATHCALL operator*(Vector4f lhs, value_type rhs) { return Vector4f{_mm_mul_ps(lhs.v, _mm_set1_ps(rhs))}; }
        friend _gm_SSE_MATHCALL operator/(Vector4f lhs, value_type rhs) { return Vector4f{_mm_div_ps(lhs.v, _mm_set1_ps(rhs))}; }

        friend _gm_SSE_MATHCALL operator+(value_type lhs, Vector4f rhs) { return Vector4f{_mm_add_ps(_mm_set1_ps(lhs), rhs.v)}; }
        friend _gm_SSE_MATHCALL operator-(value_type lhs, Vector4f rhs) { return Vector4f{_mm_sub_ps(_mm_set1_ps(lhs), rhs.v)}; }
        friend _gm_SSE_MATHCALL operator*(value_type lhs, Vector4f rhs) { return Vector4f{_mm_mul_ps(_mm_set1_ps(lhs), rhs.v)}; }
        friend _gm_SSE_MATHCALL operator/(value_type lhs, Vector4f rhs) { return Vector4f{_mm_div_ps(_mm_set1_ps(lhs), rhs.v)}; }

        friend _gm_SSE_MATHCALL operator==(Vector4f lhs, Vector4f rhs) {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) == 15;
        }
        friend _gm_SSE_MATHCALL operator!=(Vector4f lhs, Vector4f rhs) {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) != 15;
        }

        _gm_SSE_MATHCALL operator[](int index) const {
            alignas(16) value_type a[length];
            _mm_store_ps(a, v);
            return a[index];
        }

    public:
        __m128 v;
    };

    using Vector = Vector4f;

    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
    static_assert(alignof(Vector4f) == 16);
} // namespace gm

#undef _gm_SSE_MATHCALL
