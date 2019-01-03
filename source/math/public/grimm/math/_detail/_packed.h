// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#define _gm_MATH_PACKED_COMMA ,
#define _gm_MATH_PACKED_BINARY_OP2(op, result_type, join_token) \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, PackedVector rhs)->result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y}; \
    } \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, typename PackedVector::value_type rhs)->result_type { \
        return result_type{lhs.m.x op rhs join_token lhs.m.y op rhs}; \
    } \
    GM_MATHCALL_FRIEND operator op(typename PackedVector::value_type lhs, PackedVector rhs)->result_type { \
        return result_type{lhs op rhs.m.x join_token lhs op rhs.m.y}; \
    }
#define _gm_MATH_PACKED_BINARY_OP3(op, result_type, join_token) \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, PackedVector rhs)->result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y join_token lhs.m.z op rhs.m.z}; \
    } \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, typename PackedVector::value_type rhs)->result_type { \
        return result_type{lhs.m.x op rhs join_token lhs.m.y op rhs join_token lhs.m.z op rhs}; \
    } \
    GM_MATHCALL_FRIEND operator op(typename PackedVector::value_type lhs, PackedVector rhs)->result_type { \
        return result_type{lhs op rhs.m.x join_token lhs op rhs.m.y join_token lhs op rhs.m.z}; \
    }
#define _gm_MATH_PACKED_BINARY_OP4(op, result_type, join_token) \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, PackedVector rhs)->result_type { \
        return result_type{lhs.m.x op rhs.m.x join_token lhs.m.y op rhs.m.y join_token lhs.m.z op rhs.m.z join_token lhs.m.w op rhs.m.w}; \
    } \
    GM_MATHCALL_FRIEND operator op(PackedVector lhs, typename PackedVector::value_type rhs)->result_type { \
        return result_type{lhs.m.x op rhs join_token lhs.m.y op rhs join_token lhs.m.z op rhs join_token lhs.m.w op rhs}; \
    } \
    GM_MATHCALL_FRIEND operator op(typename PackedVector::value_type lhs, PackedVector rhs)->result_type { \
        return result_type{lhs op rhs.m.x join_token lhs op rhs.m.y join_token lhs op rhs.m.z join_token lhs op rhs.m.w}; \
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

        using value_type = T;
        using packed_type = _detail::PackedVectorStruct<T, length>;
        using array_type = T[length];
        using const_array_type = T const[length];

        template <int M>
        using vector_template = PackedVector<T, M>;

        /*implicit*/ GM_FORCEINLINE constexpr PackedVectorBase() : m() {}
        /*implicit*/ GM_FORCEINLINE constexpr PackedVectorBase(packed_type packed) : m(packed) {}

        GM_MATHCALL_CONVERSION operator array_type&() { return u; }
        GM_MATHCALL_CONVERSION operator const_array_type&() const { return u; }

        union {
            array_type u;
            packed_type m;
        };
    };
} // namespace gm::_detail

namespace gm {
    template <typename T>
    class PackedVector<T, 4> : public _detail::PackedVectorBase<T, 4> {
    public:
        using _detail::PackedVectorBase<T, 4>::PackedVectorBase;
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(T x, T y, T z, T w) : PackedVector{{x, y, z, w}} {}
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(PackedVector<T, 3> xyz, T w = {}) : PackedVector{xyz.m.x, xyz.m.y, xyz.m.z, w} {}

        _gm_MATH_PACKED_BINARY_OP4(+, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP4(-, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP4(*, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP4(/, PackedVector, _gm_MATH_PACKED_COMMA);

        _gm_MATH_PACKED_BINARY_OP4(==, bool, &&);
        _gm_MATH_PACKED_BINARY_OP4(!=, bool, ||);
    };

    template <typename T>
    class PackedVector<T, 3> : public _detail::PackedVectorBase<T, 3> {
    public:
        using _detail::PackedVectorBase<T, 3>::PackedVectorBase;
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(T x, T y, T z) : PackedVector{{x, y, z}} {}
        /*implicit*/ GM_FORCEINLINE constexpr PackedVector(PackedVector<T, 2> xy, T z = {}) : PackedVector{xy.m.x, xy.m.y, z} {}

        _gm_MATH_PACKED_BINARY_OP3(+, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP3(-, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP3(*, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP3(/, PackedVector, _gm_MATH_PACKED_COMMA);

        _gm_MATH_PACKED_BINARY_OP3(==, bool, &&);
        _gm_MATH_PACKED_BINARY_OP3(!=, bool, ||);
    };

    template <typename T>
    class PackedVector<T, 2> : public _detail::PackedVectorBase<T, 2> {
    public:
        using _detail::PackedVectorBase<T, 2>::PackedVectorBase;
        /*implict*/ GM_FORCEINLINE constexpr PackedVector(T x, T y) : PackedVector{{x, y}} {}

        _gm_MATH_PACKED_BINARY_OP2(+, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP2(-, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP2(*, PackedVector, _gm_MATH_PACKED_COMMA);
        _gm_MATH_PACKED_BINARY_OP2(/, PackedVector, _gm_MATH_PACKED_COMMA);

        _gm_MATH_PACKED_BINARY_OP2(==, bool, &&);
        _gm_MATH_PACKED_BINARY_OP2(!=, bool, ||);
    };

    static_assert(sizeof(PackedVector2f) == sizeof(float) * 2);
    static_assert(sizeof(PackedVector3f) == sizeof(float) * 3);
    static_assert(sizeof(PackedVector4f) == sizeof(float) * 4);
} // namespace gm

#undef _gm_MATH_PACKED_COMMA
#undef _gm_MATH_PACKED_BINARY_OP2
#undef _gm_MATH_PACKED_BINARY_OP3
#undef _gm_MATH_PACKED_BINARY_OP4
