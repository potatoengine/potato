// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    enum class format_result : unsigned int {
        success,
        out_of_range,
        malformed_input,
        out_of_space,
    };
} // namespace up
