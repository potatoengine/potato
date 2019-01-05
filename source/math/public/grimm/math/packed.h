// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

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
} // namespace gm

#include "_detail/_packed.h"
