// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/platform.h"

#if defined(GM_MATH_ENABLE_SWIZZLE)
#    define _gm_MATH_SWIZZLE2(a, b) \
        GM_MATHCALL a##b() const->PackedVector<T, 2> { \
            return {m.a, m.b}; \
        }
#    define _gm_MATH_SWIZZLE3(a, b, c) \
        GM_MATHCALL a##b##c() const->PackedVector<T, 3> { \
            return {m.a, m.b, m.c}; \
        }
#    define _gm_MATH_SWIZZLE4(a, b, c, d) \
        GM_MATHCALL a##b##c##d() const->PackedVector<T, 4> { \
            return {m.a, m.b, m.c, m.d}; \
        }
#else // defined(GM_MATH_ENABLE_SWIZZLE)
#    define _gm_MATH_SWIZZLE2(a, b)
#    define _gm_MATH_SWIZZLE3(a, b, c)
#    define _gm_MATH_SWIZZLE4(a, b, c, d)
#endif // defined(GM_MATH_ENABLE_SWIZZLE)

#define _gm_MATH_COMMA ,
#define _gm_MATH_BINARY_OP2(op, result_type, join_token) \
    friend GM_MATHCALL operator op(PackedVector lhs, PackedVector rhs) -> result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y}; \
    }
#define _gm_MATH_BINARY_OP3(op, result_type, join_token) \
    friend GM_MATHCALL operator op(PackedVector lhs, PackedVector rhs)->result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y join_token lhs.m.z op rhs.m.z}; \
    }
#define _gm_MATH_BINARY_OP4(op, result_type, join_token) \
    friend GM_MATHCALL operator op(PackedVector lhs, PackedVector rhs)->result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y join_token lhs.m.z op rhs.m.z join_token lhs.m.w op rhs.m.w}; \
    }

namespace gm::_detail {
    template <typename T, int N>
    struct PackedVectorStruct;

    template <typename T>
    struct PackedVectorStruct<T, 4> {
        T x = T{0};
        T y = T{0};
        T z = T{0};
        T w = T{0};
    };

    template <typename T>
    struct PackedVectorStruct<T, 3> {
        T x = T{0};
        T y = T{0};
        T z = T{0};
    };

    template <typename T>
    struct PackedVectorStruct<T, 2> {
        T x = T{0};
        T y = T{0};
    };

    template <typename T, int N>
    struct PackedVectorBase {
        static constexpr int length = N;

        using packed_type = _detail::PackedVectorStruct<T, length>;
        using array_type = T[length];
        using const_array_type = T const[length];

        /*implicit*/ GM_FORCEINLINE constexpr PackedVectorBase() : m() {}
        /*implicit*/ GM_FORCEINLINE constexpr PackedVectorBase(packed_type packed) : m(packed) {}

        GM_MATHCALL_CONVERSION operator array_type&() { return u; }
        GM_MATHCALL_CONVERSION operator const_array_type&() const { return u; }

        union {
            T u[length];
            packed_type m;
        };
    };
} // namespace gm::_detail

namespace gm {
    template <typename T, int N>
    class PackedVector;

    template <typename T>
    PackedVector(T x, T y)->PackedVector<T, 2>;
    template <typename T>
    PackedVector(T x, T y, T z)->PackedVector<T, 3>;
    template <typename T>
    PackedVector(T x, T y, T z, T w)->PackedVector<T, 4>;

    using PackedVector2f = PackedVector<float, 2>;
    using PackedVector3f = PackedVector<float, 3>;
    using PackedVector4f = PackedVector<float, 4>;

    template <typename T>
    class PackedVector<T, 4> : public _detail::PackedVectorBase<T, 4> {
    public:
        using _detail::PackedVectorBase<T, 4>::PackedVectorBase;
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(T x, T y, T z, T w) : PackedVector{{x, y, z, w}} {
        }
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(PackedVector<T, 3> xyz, T w = {}) : PackedVector{xyz.m.x, xyz.m.y, xyz.m.z, w} {}

        _gm_MATH_BINARY_OP4(+, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP4(-, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP4(*, PackedVector, _gm_MATH_COMMA);

        _gm_MATH_BINARY_OP4(==, bool, &&);
        _gm_MATH_BINARY_OP4(!=, bool, ||);

        _gm_MATH_SWIZZLE4(w, z, y, x);
        _gm_MATH_SWIZZLE3(x, y, z);
        _gm_MATH_SWIZZLE2(x, y);
        _gm_MATH_SWIZZLE2(y, x);
    };

    template <typename T>
    class PackedVector<T, 3> : public _detail::PackedVectorBase<T, 3> {
    public:
        using _detail::PackedVectorBase<T, 3>::PackedVectorBase;
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(T x, T y, T z) : PackedVector{{x, y, z}} {
        }

        _gm_MATH_BINARY_OP3(+, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP3(-, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP3(*, PackedVector, _gm_MATH_COMMA);

        _gm_MATH_BINARY_OP3(==, bool, &&);
        _gm_MATH_BINARY_OP3(!=, bool, ||);

        _gm_MATH_SWIZZLE3(z, y, x);
        _gm_MATH_SWIZZLE2(x, y);
    };

    template <typename T>
    class PackedVector<T, 2> : public _detail::PackedVectorBase<T, 2> {
    public:
        using _detail::PackedVectorBase<T, 2>::PackedVectorBase;
        /*implict*/ GM_FORCEINLINE constexpr PackedVector(T x, T y) : PackedVector{{x, y}} {
        }

        _gm_MATH_BINARY_OP2(+, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP2(-, PackedVector, _gm_MATH_COMMA);
        _gm_MATH_BINARY_OP2(*, PackedVector, _gm_MATH_COMMA);

        _gm_MATH_BINARY_OP2(==, bool, &&);
        _gm_MATH_BINARY_OP2(!=, bool, ||);

        _gm_MATH_SWIZZLE2(y, x);
    };

    static_assert(sizeof(PackedVector2f) == sizeof(float) * 2);
    static_assert(sizeof(PackedVector3f) == sizeof(float) * 3);
    static_assert(sizeof(PackedVector4f) == sizeof(float) * 4);
} // namespace gm

#undef _gm_MATH_SWIZZLE2
#undef _gm_MATH_SWIZZLE3
#undef _gm_MATH_SWIZZLE4
#undef _gm_MATH_COMMA
#undef _gm_MATH_BINARY_OP2
#undef _gm_MATH_BINARY_OP3
#undef _gm_MATH_BINARY_OP4
