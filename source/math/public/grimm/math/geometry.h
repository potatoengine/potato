// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/math/vector.h"

#include <cmath>

namespace gm {
    auto GM_VECTORCALL rotationXY(float radians) noexcept -> Matrix4f;
    auto GM_VECTORCALL rotationXZ(float radians) noexcept -> Matrix4f;
    auto GM_VECTORCALL rotationYZ(float radians) noexcept -> Matrix4f;

    auto GM_VECTORCALL projection(float aspect, float fovY, float nearZ, float farZ) noexcept -> Matrix4f;

} // namespace gm
