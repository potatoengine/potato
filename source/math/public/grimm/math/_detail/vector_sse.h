// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/platform.h"
#include "grimm/math/common.h"

#if !defined(GM_ARCH_INTEL)
#    error "SSE can only be used on Intel architectures"
#endif

#include <cmath>
#include <cstdint>
#include <limits>

#include <pmmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>

// SSE vec4tor modeled on: http://www.codersnotes.com/notes/maths-lib-2016/

namespace gm {
    struct Vec4 {
        Vec4() noexcept : v(_mm_set1_ps(0.f)) {}
        /*implicit*/ Vec4(noinit_t) noexcept {}
        /*implicit*/ Vec4(float x, float y, float z, float w) noexcept : v(_mm_set_ps(w, z, y, x)) {}
        /*implicit*/ Vec4(vec_broadcast_t, float value) noexcept : v(_mm_set1_ps(value)) {}
        /*implicit*/ Vec4(__m128 m) : v(m) {}

        GM_FORCEINLINE static auto GM_VECTORCALL unalignedLoad(const float* unaligned) noexcept -> Vec4 {
            return _mm_loadu_ps(unaligned);
        }
        GM_FORCEINLINE static auto GM_VECTORCALL alignedLoad(const float* aligned) noexcept -> Vec4 {
            return _mm_load_ps(aligned);
        }

        GM_FORCEINLINE auto GM_VECTORCALL unalignedStore(float* unaligned) noexcept -> void {
            _mm_storeu_ps(unaligned, v);
        }
        GM_FORCEINLINE auto GM_VECTORCALL alignedStore(float* aligned) noexcept -> void {
            _mm_store_ps(aligned, v);
        }

        template <int X = 0, int Y = 1, int Z = 2, int W = 3>
        GM_FORCEINLINE auto GM_VECTORCALL shuffle() const noexcept { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W, Z, Y, X)); }

        GM_FORCEINLINE auto GM_VECTORCALL x() const noexcept -> float { return _mm_cvtss_f32(v); }
        GM_FORCEINLINE auto GM_VECTORCALL y() const noexcept -> float { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1))); }
        GM_FORCEINLINE auto GM_VECTORCALL z() const noexcept -> float { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))); }
        GM_FORCEINLINE auto GM_VECTORCALL w() const noexcept -> float { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))); }

        GM_FORCEINLINE auto GM_VECTORCALL x(float f) noexcept -> float {
            __m128 t = _mm_unpacklo_ps(_mm_set1_ps(f), v);
            v = _mm_shuffle_ps(t, v, _MM_SHUFFLE(0, 3, 2, 3));
            return f;
        }
        GM_FORCEINLINE auto GM_VECTORCALL y(float f) noexcept -> float {
            __m128 t = _mm_unpacklo_ps(_mm_set1_ps(f), v);
            v = _mm_shuffle_ps(t, v, _MM_SHUFFLE(1, 0, 2, 3));
            return f;
        }
        GM_FORCEINLINE auto GM_VECTORCALL z(float f) noexcept -> float {
            __m128 t = _mm_unpackhi_ps(_mm_set1_ps(f), v);
            v = _mm_shuffle_ps(t, v, _MM_SHUFFLE(0, 1, 0, 3));
            return f;
        }
        GM_FORCEINLINE auto GM_VECTORCALL w(float f) noexcept -> float {
            __m128 t = _mm_unpackhi_ps(_mm_set1_ps(f), v);
            v = _mm_shuffle_ps(t, v, _MM_SHUFFLE(0, 1, 1, 0));
            return f;
        }

        GM_FORCEINLINE auto GM_VECTORCALL operator[](int index) const -> float {
            alignas(16) float a[4];
            _mm_store_ps(a, v);
            return a[index];
        }

        __m128 v;
    };

    template <>
    struct vector_size<Vec4> {
        static constexpr bool is_vector = true;
        static constexpr int size = 4;
        template <int M>
        using resize_t = Vec4;
    };

    template <int A, int B, int C = 2, int D = 3>
    Vec4 shuffle(Vec4 val) noexcept { return Vec4{_mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(D, C, B, A))}; }

    GM_FORCEINLINE inline auto GM_VECTORCALL operator+(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_add_ps(lhs.v, rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator-(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_sub_ps(lhs.v, rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator*(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_mul_ps(lhs.v, rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator/(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_div_ps(lhs.v, rhs.v); }

    GM_FORCEINLINE inline auto GM_VECTORCALL operator+(Vec4 lhs, float rhs) noexcept -> Vec4 { return _mm_add_ps(lhs.v, _mm_set1_ps(rhs)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator-(Vec4 lhs, float rhs) noexcept -> Vec4 { return _mm_sub_ps(lhs.v, _mm_set1_ps(rhs)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator*(Vec4 lhs, float rhs) noexcept -> Vec4 { return _mm_mul_ps(lhs.v, _mm_set1_ps(rhs)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator/(Vec4 lhs, float rhs) noexcept -> Vec4 { return _mm_div_ps(lhs.v, _mm_set1_ps(rhs)); }

    GM_FORCEINLINE inline auto GM_VECTORCALL operator+(float lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_add_ps(_mm_set1_ps(lhs), rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator-(float lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_sub_ps(_mm_set1_ps(lhs), rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator*(float lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_mul_ps(_mm_set1_ps(lhs), rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator/(float lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_div_ps(_mm_set1_ps(lhs), rhs.v); }

    GM_FORCEINLINE inline auto GM_VECTORCALL operator-(Vec4 val) noexcept -> Vec4 { return _mm_sub_ps(_mm_setzero_ps(), val.v); }

    GM_FORCEINLINE inline auto GM_VECTORCALL operator==(Vec4 lhs, Vec4 rhs) noexcept -> bool {
        return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) == 15;
    }
    GM_FORCEINLINE inline auto GM_VECTORCALL operator!=(Vec4 lhs, Vec4 rhs) noexcept -> bool {
        return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) != 15;
    }

    GM_FORCEINLINE inline auto GM_VECTORCALL min(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_min_ps(lhs.v, rhs.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL max(Vec4 lhs, Vec4 rhs) noexcept -> Vec4 { return _mm_max_ps(lhs.v, rhs.v); }

    GM_FORCEINLINE inline auto GM_VECTORCALL abs(Vec4 val) noexcept -> Vec4 {
        auto signBits = _mm_set_ps(0x80000000, 0x80000000, 0x80000000, 0x80000000);
        return _mm_andnot_ps(signBits, val.v);
    }

    GM_FORCEINLINE inline auto GM_VECTORCALL hsum(Vec4 val) {
        __m128 t = _mm_hadd_ps(val.v, val.v);
        return _mm_cvtss_f32(_mm_hadd_ps(t, t));
    }
    GM_FORCEINLINE inline auto GM_VECTORCALL hmin(Vec4 val) {
        __m128 a = _mm_min_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(1, 3, 0, 2)));
        __m128 b = _mm_min_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(2, 0, 3, 1)));
        __m128 c = _mm_min_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(0, 1, 2, 3)));
        return _mm_cvtss_f32(_mm_min_ps(_mm_min_ps(a, b), c));
    }
    GM_FORCEINLINE inline auto GM_VECTORCALL hmax(Vec4 val) {
        __m128 a = _mm_max_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(1, 3, 0, 2)));
        __m128 b = _mm_max_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(2, 0, 3, 1)));
        __m128 c = _mm_max_ps(val.v, _mm_shuffle_ps(val.v, val.v, _MM_SHUFFLE(0, 1, 2, 3)));
        return _mm_cvtss_f32(_mm_max_ps(_mm_max_ps(a, b), c));
    }

    GM_FORCEINLINE inline auto GM_VECTORCALL sqrt(Vec4 val) { return _mm_sqrt_ps(val.v); }

    GM_FORCEINLINE inline auto GM_VECTORCALL clamp(Vec4 t, Vec4 a, Vec4 b) noexcept -> Vec4 { return min(max(t, a), b); }
    GM_FORCEINLINE inline auto GM_VECTORCALL dot(Vec4 a, Vec4 b) { return _mm_cvtss_f32(_mm_dp_ps(a.v, b.v, 0xff)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL dot3(Vec4 a, Vec4 b) { return _mm_cvtss_f32(_mm_dp_ps(a.v, b.v, 0x77)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL length(Vec4 val) noexcept -> float { return std::sqrt(dot(val, val)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL length3(Vec4 val) noexcept -> float { return std::sqrt(dot3(val, val)); }
    GM_FORCEINLINE inline auto GM_VECTORCALL reciprocal(Vec4 val) noexcept -> Vec4 { return _mm_rcp_ps(val.v); }
    GM_FORCEINLINE inline auto GM_VECTORCALL normalize(Vec4 val) noexcept -> Vec4 { return val * reciprocal(Vec4{vec_broadcast, length(val)}); }
    GM_FORCEINLINE inline auto GM_VECTORCALL normalize3(Vec4 val) noexcept -> Vec4 { return val * reciprocal(Vec4{vec_broadcast, length3(val)}); }
    GM_FORCEINLINE inline auto GM_VECTORCALL lerp(Vec4 a, Vec4 b, float t) noexcept -> Vec4 { return a + (b - a) * t; }

    GM_FORCEINLINE inline auto GM_VECTORCALL cross(Vec4 a, Vec4 b) {
        // x  <-  a.y*b.z - a.z*b.y
        // y  <-  a.z*b.x - a.x*b.z
        // z  <-  a.x*b.y - a.y*b.x
        // We can save a shuffle by grouping it in this wacky order:
        __m128 as = _mm_shuffle_ps(a.v, a.v, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 bs = _mm_shuffle_ps(b.v, b.v, _MM_SHUFFLE(3, 1, 0, 2));
        __m128 t = _mm_sub_ps(_mm_mul_ps(as, b.v), _mm_mul_ps(a.v, bs));
        return _mm_shuffle_ps(t, t, _MM_SHUFFLE(3, 1, 0, 2));
    }
} // namespace gm
