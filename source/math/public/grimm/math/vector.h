// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"
#include "packed.h"

namespace gm {
    // FIXME: make this a SIMD vector
    using Vector4f = PackedVector4f;
    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
} // namespace gm
