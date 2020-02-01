// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

namespace up {
    enum class format_result : unsigned int {
        success,
        out_of_range,
        malformed_input,
        out_of_space,
    };
} // up
