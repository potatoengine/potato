// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/math/vector.h"

#include <cmath>

namespace gm {
    GM_FORCEINLINE auto GM_VECTORCALL rotationXY(float radians) noexcept -> Matrix4f {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return {
            {c, s, 0, 0},
            {-s, c, 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}};
    }

    GM_FORCEINLINE auto GM_VECTORCALL rotationXZ(float radians) noexcept -> Matrix4f {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return {
            {c, 0, s, 0},
            {0, 1, 0, 0},
            {-s, 0, c, 0},
            {0, 0, 0, 1}};
    }

    GM_FORCEINLINE auto GM_VECTORCALL rotationYZ(float radians) noexcept -> Matrix4f {
        float c = std::cos(radians);
        float s = std::sin(radians);
        return {
            {1, 0, 0, 0},
            {0, c, -s, 0},
            {0, s, c, 0},
            {0, 0, 0, 1}};
    }

} // namespace gm
