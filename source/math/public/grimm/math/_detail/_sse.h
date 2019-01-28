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

#define _gm_SSE_MATHCALL GM_FORCEINLINE auto GM_VECTORCALL
#define _gm_SSE_MATHCALL_FRIEND GM_FORCEINLINE friend auto GM_VECTORCALL
#define _gm_SSE_MATHCALL_STATIC GM_FORCEINLINE static auto GM_VECTORCALL
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

        _gm_SSE_MATHCONSTRUCTOR Vector4f() noexcept : v(_mm_set1_ps(0.f)) {}
        /*implicit*/ _gm_SSE_MATHCONSTRUCTOR Vector4f(noinit_t) noexcept {}
        /*implicit*/ _gm_SSE_MATHCONSTRUCTOR Vector4f(value_type x, value_type y, value_type z, value_type w) noexcept : v(_mm_set_ps(w, z, y, x)) {}
        /*implicit*/ _gm_SSE_MATHCONSTRUCTOR Vector4f(vec_broadcast_t, value_type value) noexcept : v(_mm_set1_ps(value)) {}
        /*implicit*/ Vector4f(__m128 vec) : v(vec) {}

        _gm_SSE_MATHCALL_STATIC unalignedLoad(const value_type* unaligned) noexcept->Vector4f {
            return _mm_loadu_ps(unaligned);
        }
        _gm_SSE_MATHCALL_STATIC alignedLoad(const value_type* aligned) noexcept->Vector4f {
            return _mm_load_ps(aligned);
        }

        _gm_SSE_MATHCALL unalignedStore(value_type* unaligned) noexcept->void {
            _mm_storeu_ps(unaligned, v);
        }
        _gm_SSE_MATHCALL alignedStore(value_type* aligned) noexcept->void {
            _mm_store_ps(aligned, v);
        }

        template <int X = 0, int Y = 1, int Z = 2, int W = 3>
        _gm_SSE_MATHCALL shuffle() const noexcept { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(W, Z, Y, X)); }

        _gm_SSE_MATHCALL x() const noexcept->value_type { return _mm_cvtss_f32(v); }
        _gm_SSE_MATHCALL y() const noexcept->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(1, 1, 1, 1))); }
        _gm_SSE_MATHCALL z() const noexcept->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 2, 2, 2))); }
        _gm_SSE_MATHCALL w() const noexcept->value_type { return _mm_cvtss_f32(_mm_shuffle_ps(v, v, _MM_SHUFFLE(3, 3, 3, 3))); }

        _gm_SSE_MATHCALL wzyx() const noexcept->Vector4f { return _mm_shuffle_ps(v, v, _MM_SHUFFLE(0, 1, 2, 3)); }

        _gm_SSE_MATHCALL_FRIEND operator+(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_add_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_sub_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator*(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_mul_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator/(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_div_ps(lhs.v, rhs.v); }

        _gm_SSE_MATHCALL_FRIEND operator+(Vector4f lhs, value_type rhs) noexcept->Vector4f { return _mm_add_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f lhs, value_type rhs) noexcept->Vector4f { return _mm_sub_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator*(Vector4f lhs, value_type rhs) noexcept->Vector4f { return _mm_mul_ps(lhs.v, _mm_set1_ps(rhs)); }
        _gm_SSE_MATHCALL_FRIEND operator/(Vector4f lhs, value_type rhs) noexcept->Vector4f { return _mm_div_ps(lhs.v, _mm_set1_ps(rhs)); }

        _gm_SSE_MATHCALL_FRIEND operator+(value_type lhs, Vector4f rhs) noexcept->Vector4f { return _mm_add_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator-(value_type lhs, Vector4f rhs) noexcept->Vector4f { return _mm_sub_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator*(value_type lhs, Vector4f rhs) noexcept->Vector4f { return _mm_mul_ps(_mm_set1_ps(lhs), rhs.v); }
        _gm_SSE_MATHCALL_FRIEND operator/(value_type lhs, Vector4f rhs) noexcept->Vector4f { return _mm_div_ps(_mm_set1_ps(lhs), rhs.v); }

        _gm_SSE_MATHCALL_FRIEND operator-(Vector4f vec) noexcept->Vector4f { return _mm_sub_ps(_mm_setzero_ps(), vec.v); }

        _gm_SSE_MATHCALL_FRIEND operator==(Vector4f lhs, Vector4f rhs) noexcept->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) == 15;
        }
        _gm_SSE_MATHCALL_FRIEND operator!=(Vector4f lhs, Vector4f rhs) noexcept->bool {
            return (_mm_movemask_ps(_mm_cmpeq_ps(lhs.v, rhs.v)) & 15) != 15;
        }

        _gm_SSE_MATHCALL operator[](int index) const->value_type {
            alignas(16) value_type a[component_length];
            _mm_store_ps(a, v);
            return a[index];
        }

        _gm_SSE_MATHCALL_FRIEND min(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_min_ps(lhs.v, rhs.v); }
        _gm_SSE_MATHCALL_FRIEND max(Vector4f lhs, Vector4f rhs) noexcept->Vector4f { return _mm_max_ps(lhs.v, rhs.v); }

        _gm_SSE_MATHCALL_FRIEND abs(Vector4f vec) noexcept->Vector4f {
            auto signBits = _mm_set_ps(0x80000000, 0x80000000, 0x80000000, 0x80000000);
            return _mm_andnot_ps(signBits, vec.v);
        }

        _gm_SSE_MATHCALL_FRIEND clamp(Vector4f t, Vector4f a, Vector4f b) noexcept->Vector4f { return min(max(t, a), b); }
        _gm_SSE_MATHCALL_FRIEND horizontalAdd(Vector4f vec) noexcept->value_type {
            __m128 t = _mm_hadd_ps(vec.v, vec.v);
            return _mm_cvtss_f32(_mm_hadd_ps(t, t));
        }
        _gm_SSE_MATHCALL_FRIEND dot(Vector4f lhs, Vector4f rhs) noexcept->value_type { return horizontalAdd(lhs * rhs); }
        _gm_SSE_MATHCALL_FRIEND squareLength(Vector4f vec) noexcept->value_type { return dot(vec, vec); }
        _gm_SSE_MATHCALL_FRIEND length(Vector4f vec) noexcept->value_type { return std::sqrt(dot(vec, vec)); }
        _gm_SSE_MATHCALL_FRIEND reciprocal(Vector4f vec) noexcept->Vector4f { return _mm_rcp_ps(vec.v); }
        _gm_SSE_MATHCALL_FRIEND normalize(Vector4f vec) noexcept->Vector4f { return vec * reciprocal(Vector4f{vec_broadcast, length(vec)}); }
        _gm_SSE_MATHCALL_FRIEND lerp(Vector4f a, Vector4f b, value_type t) noexcept->Vector4f { return a + (b - a) * t; }

    public:
        __m128 v;
    };

    class Matrix4f {
    public:
        static constexpr int component_length = 4;

        using value_type = Vector4f;
        using component_type = typename Vector4f::value_type;
        using const_array_type = value_type const[component_length];

        _gm_SSE_MATHCONSTRUCTOR Matrix4f() noexcept : c{{1.f, 0.f, 0.f, 0.f}, {0.f, 1.f, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}} {}
        /*implicit*/ _gm_SSE_MATHCONSTRUCTOR Matrix4f(noinit_t) noexcept {}
        /*implicit*/ _gm_SSE_MATHCONSTRUCTOR Matrix4f(value_type c0, value_type c1, value_type c2, value_type c3) noexcept : c{c0, c1, c2, c3} {}

        _gm_SSE_MATHCALL_STATIC unalignedLoad(const component_type* unaligned) noexcept->Matrix4f {
            return {
                _mm_loadu_ps(unaligned + 0),
                _mm_loadu_ps(unaligned + 4),
                _mm_loadu_ps(unaligned + 8),
                _mm_loadu_ps(unaligned + 12)};
        }
        _gm_SSE_MATHCALL_STATIC alignedLoad(const component_type* aligned) noexcept->Matrix4f {
            return {
                _mm_load_ps(aligned + 0),
                _mm_load_ps(aligned + 4),
                _mm_load_ps(aligned + 8),
                _mm_load_ps(aligned + 12)};
        }

        _gm_SSE_MATHCALL unalignedStore(component_type* unaligned) noexcept->void {
            _mm_storeu_ps(unaligned + 0, c[0].v);
            _mm_storeu_ps(unaligned + 4, c[1].v);
            _mm_storeu_ps(unaligned + 8, c[2].v);
            _mm_storeu_ps(unaligned + 12, c[3].v);
        }
        _gm_SSE_MATHCALL alignedStore(component_type* aligned) noexcept {
            _mm_store_ps(aligned + 0, c[0].v);
            _mm_store_ps(aligned + 4, c[1].v);
            _mm_store_ps(aligned + 8, c[2].v);
            _mm_store_ps(aligned + 12, c[3].v);
        }

        friend auto GM_VECTORCALL operator*(Matrix4f lhs, Matrix4f rhs) noexcept -> Matrix4f;
        auto GM_VECTORCALL operator*=(Matrix4f rhs) noexcept -> Matrix4f&;

        friend auto GM_VECTORCALL operator*(Matrix4f lhs, value_type rhs) noexcept -> value_type;

    public:
        value_type c[4];
    };

    auto GM_VECTORCALL transpose(Matrix4f mat) noexcept -> Matrix4f;
    auto GM_VECTORCALL transformInverseUnscaled(Matrix4f mat) noexcept -> Matrix4f;
    auto GM_VECTORCALL transformInverse(Matrix4f mat) noexcept -> Matrix4f;

    using Vector = Vector4f;
    using Matrix = Matrix4f;

    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
    static_assert(alignof(Vector4f) == 16);

    static_assert(sizeof(Matrix4f) == sizeof(float) * 16);
    static_assert(alignof(Matrix4f) == 16);
} // namespace gm

#undef _gm_SSE_MATHCALL
#undef _gm_SSE_MATHCALL_FRIEND
#undef _gm_SSE_MATHCALL_STATIC
#undef _gm_SSE_MATHCONSTRUCTOR
