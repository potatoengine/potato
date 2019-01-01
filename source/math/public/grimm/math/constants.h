// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

namespace gm::constants {
    template <typename T>
    constexpr T pi = (T)3.14159265358979323846;

    template <typename T>
    constexpr T halfPi = pi<T> / T{2};

    template <typename T>
    constexpr T degreesToRadians = pi<T> / T{180};

    template <typename T>
    constexpr T radiansToDegrees = T{180} / pi<T>;

    
} // namespace gm
