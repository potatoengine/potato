// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    constexpr char const* format_parse_nonnegative(char const* start, char const* end, int& result) noexcept {
        // std::from_chars is not (yet) constexpr
        result = 0;
        while (start != end && *start >= '0' && *start <= '9') {
            result *= 10;
            result += *start - '0';
            ++start;
        }
        return start;
    }
} // namespace up
