// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "geometry.h"

auto GM_VECTORCALL gm::rotationXY(float radians) noexcept -> Matrix4f {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return {
        {c, s, 0, 0},
        {-s, c, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::rotationXZ(float radians) noexcept -> Matrix4f {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return {
        {c, 0, s, 0},
        {0, 1, 0, 0},
        {-s, 0, c, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::rotationYZ(float radians) noexcept -> Matrix4f {
    float c = std::cos(radians);
    float s = std::sin(radians);
    return {
        {1, 0, 0, 0},
        {0, c, -s, 0},
        {0, s, c, 0},
        {0, 0, 0, 1}};
}

auto GM_VECTORCALL gm::projection(float aspect, float fovY, float nearZ, float farZ) noexcept -> Matrix4f {
    float height = 1 / std::tan(fovY * 0.5f);
    float width = height / aspect;
    float range = farZ / (nearZ - farZ);
    float a = farZ / (farZ - nearZ);
    float b = (-nearZ * farZ) / (farZ - nearZ);

    return {
        {width, 0, 0, 0},
        {0, height, 0, 0},
        {0, 0, range, nearZ * range},
        {0, 0, -1, 0}};
}
