// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "matrix.h"
#include <cmath>

auto GM_VECTORCALL gm::rotationZ(float radians) noexcept -> Mat4x4 {
    float r = std::cos(radians);
    float s = std::sin(radians);
    return {
        {r, s, 0, 0},
        {-s, r, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::rotationY(float radians) noexcept -> Mat4x4 {
    float r = std::cos(radians);
    float s = std::sin(radians);
    return {
        {r, 0, s, 0},
        {0, 1, 0, 0},
        {-s, 0, r, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::rotationX(float radians) noexcept -> Mat4x4 {
    float r = std::cos(radians);
    float s = std::sin(radians);
    return {
        {1, 0, 0, 0},
        {0, r, -s, 0},
        {0, s, r, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::projection(float aspect, float fovY, float nearZ, float farZ) noexcept -> Mat4x4 {
    float height = 1 / std::tan(fovY * 0.5f);
    float width = height / aspect;
    float range = farZ / (nearZ - farZ);

    return {
        {width, 0, 0, 0},
        {0, height, 0, 0},
        {0, 0, range, nearZ * range},
        {0, 0, -1, 0}};
}
