// Copyright (C) 2018 Sean Middleditch, all rights reserverd.

#pragma once

#include "common.h"

namespace gm {
    struct Vector4f {
        float x = 0;
        float y = 0;
        float z = 0;
        float w = 0;
    };
    static_assert(sizeof(Vector4f) == sizeof(float) * 4);
}
