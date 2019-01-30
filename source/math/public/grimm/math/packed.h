// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "grimm/foundation/types.h"
#include <initializer_list>

namespace gm::_detail {
    template <typename T, int N>
    struct PackedBase;

    template <typename T>
    struct PackedBase<T, 4> {
        PackedBase() = default;
        PackedBase(T a, T b, T c, T d) noexcept : x(a), y(b), z(c), w(d) {}

        T x = T{0};
        T y = T{0};
        T z = T{0};
        T w = T{0};
    };

    template <typename T>
    struct PackedBase<T, 3> {
        PackedBase() = default;
        PackedBase(T a, T b, T c) noexcept : x(a), y(b), z(c) {}

        T x = T{0};
        T y = T{0};
        T z = T{0};
    };

    template <typename T>
    struct PackedBase<T, 2> {
        PackedBase() = default;
        PackedBase(T a, T b) noexcept : x(a), y(b) {}

        T x = T{0};
        T y = T{0};
    };
} // namespace gm::_detail

namespace gm {
    template <typename T, int N>
    struct Packed : _detail::PackedBase<T, N> {
        using _detail::PackedBase<T, N>::PackedBase;

        T& operator[](int i) noexcept { return (&this->x)[i]; }
        T operator[](int i) const noexcept { return (&this->x)[i]; }
    };

    template <typename T>
    Packed(T, T)->Packed<T, 2>;
    template <typename T>
    Packed(T, T, T)->Packed<T, 3>;
    template <typename T>
    Packed(T, T, T, T)->Packed<T, 4>;

    template <typename T, int N>
    constexpr int component_length_v<Packed<T, N>> = N;

    template <typename T, int N>
    constexpr bool is_vector_v<Packed<T, N>> = true;

    template <typename T, int N>
    struct vector_size<Packed<T, N>> {
        static constexpr bool is_vector = true;
        static constexpr int size = N;
        template <int M>
        using resize_t = Packed<T, M>;
    };

    template <int A, int B, int C, int D, typename T, int N>
    Packed<T, 4> shuffle(Packed<T, N> p) noexcept { return {p[A], p[B], p[C], p[D]}; }
    template <int A, int B, int C, typename T, int N>
    Packed<T, 3> shuffle(Packed<T, N> p) noexcept { return {p[A], p[B], p[C]}; }
    template <int A, int B, typename T, int N>
    Packed<T, 2> shuffle(Packed<T, N> p) noexcept { return {p[A], p[B]}; }

    using Packed2 = Packed<float, 2>;
    using Packed3 = Packed<float, 3>;
    using Packed4 = Packed<float, 4>;

    template <typename T, int N>
    auto GM_VECTORCALL operator==(Packed<T, N> lhs, Packed<T, N> rhs) noexcept -> bool {
        for (int i = 0; i != N; ++i) {
            if (lhs[i] != rhs[i]) {
                return false;
            }
        }
        return true;
    }
    template <typename T, int N>
    auto GM_VECTORCALL operator!=(Packed<T, N> lhs, Packed<T, N> rhs) noexcept { return !(lhs == rhs); }

} // namespace gm
