// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.
#pragma once

#include "potato/spud/ascii.h"

namespace up::_detail {

    // std::from_chars is not (yet) constexpr
    template <typename T>
    constexpr char const* parse_unsigned(char const* start, char const* end, T& result) noexcept {
        if (start != end && ascii::is_digit(*start)) {
            result = 0;
            do {
                result *= 10;
                result += *start - '0';
                ++start;
            } while (start != end && ascii::is_digit(*start));
        }
        return start;
    }

} // namespace up::_detail
