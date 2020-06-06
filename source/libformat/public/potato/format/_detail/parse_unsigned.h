// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up::_detail {

    constexpr bool is_digit(char ch) noexcept { return ch >= '0' && ch <= '9'; }

    // std::from_chars is not (yet) constexpr
    constexpr char const* parse_unsigned(char const* start, char const* end, unsigned& result) noexcept {
        if (start != end && is_digit(*start)) {
            result = 0;
            do {
                result *= 10; // NOLINT(readability-magic-numbers)
                result += *start - '0';
                ++start;
            } while (start != end && is_digit(*start));
        }
        return start;
    }

} // namespace up::_detail
