// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up::_detail {
    // std::from_chars is not (yet) constexpr
    template <typename T>
    constexpr char const* parse_unsigned(char const* start, char const* end, T& result) noexcept {
        result = 0;
        while (start != end && *start >= '0' && *start <= '9') {
            result *= 10;
            result += *start - '0';
            ++start;
        }
        return start;
    }
} // namespace up::_detail
