// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "packed.h"

namespace gm {
#define _gm_PACK_OP(op) \
    for (int i = 0; i != N; ++i) { \
        lhs[i] op rhs[i]; \
    } \
    return lhs;
#define _gm_PACK_OP_SCALAR(op) \
    for (int i = 0; i != N; ++i) { \
        lhs[i] op rhs; \
    } \
    return lhs;

    // --- Packed& @= (Packed&, Packed) ---

    template <typename T, int N>
    auto GM_VECTORCALL operator+=(Packed<T, N>& lhs, Packed<T, N> rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP(+=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator*=(Packed<T, N>& lhs, Packed<T, N> rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP(*=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator-=(Packed<T, N>& lhs, Packed<T, N> rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP(-=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator/=(Packed<T, N>& lhs, Packed<T, N> rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP(/=) }

    // --- Packed& @= (Packed&, scalar) ---

    template <typename T, int N>
    auto GM_VECTORCALL operator+=(Packed<T, N>& lhs, T rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP_SCALAR(+=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator*=(Packed<T, N>& lhs, T rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP_SCALAR(*=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator-=(Packed<T, N>& lhs, T rhs) noexcept -> Packed<T, N>& { _gm_PACK_OP_SCALAR(-=) }

    template <typename T, int N>
    auto GM_VECTORCALL operator/=(Packed<T, N>& lhs, T rhs) noexcept -> Packed<T, N>& {
        if constexpr (std::is_floating_point_v<T>) {
            T inv = T{1} / lhs;
            return rhs *= inv;
        }
        else {
            _gm_PACK_OP_SCALAR(/=)
        }
    }

    // --- Packed @ (Packed, Packed) ---

    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator+(Packed<T, N> lhs, Packed<T, N> rhs) noexcept { return lhs += rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator*(Packed<T, N> lhs, Packed<T, N> rhs) noexcept { return lhs *= rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator-(Packed<T, N> lhs, Packed<T, N> rhs) noexcept { return lhs -= rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator/(Packed<T, N> lhs, Packed<T, N> rhs) noexcept { return lhs /= rhs; }

    // --- Packed @ (Packed, scalar) ---

    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator+(Packed<T, N> lhs, T rhs) noexcept { return lhs += rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator*(Packed<T, N> lhs, T rhs) noexcept { return lhs *= rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator-(Packed<T, N> lhs, T rhs) noexcept { return lhs -= rhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator/(Packed<T, N> lhs, T rhs) noexcept { return lhs /= rhs; }

    // --- Packed @ (scalar, Packed)

    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator*(T lhs, Packed<T, N> rhs) noexcept { return rhs *= lhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator+(T lhs, Packed<T, N> rhs) noexcept { return rhs += lhs; }
    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator-(T lhs, Packed<T, N> rhs) noexcept { return rhs += -lhs; }

    template <typename T, int N>
    Packed<T, N> GM_VECTORCALL operator/(T lhs, Packed<T, N> rhs) noexcept {
        for (int i = 0; i != N; ++i) {
            rhs[i] = lhs / rhs[i];
        }
        return rhs;
    }

} // namespace gm
