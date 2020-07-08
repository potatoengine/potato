// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_spec.h"

#include <cstdio>

namespace up::_detail {

    inline int write_float_helper(char* buf, size_t buf_size, double value, string_view spec_string) noexcept {
        bool leading_zeroes = false;
        if (auto const [success, lead_zeroes] = _detail::parse_spec(spec_string, "0"); success) {
            leading_zeroes = true;
        }

        int width = 0;
        int precision = -1;
        if (auto const [success, spec_width, spec_precision] = _detail::parse_width_and_precision(spec_string);
            success) {
            width = static_cast<int>(spec_width);
            precision = static_cast<int>(spec_precision);
        }

        char printf_spec = 'f';
        if (auto const [success, spec] = parse_spec(spec_string, "aefg"); success) {
            printf_spec = spec;
        }

        constexpr std::size_t fmt_buf_size = 8;
        char fmt_buf[fmt_buf_size] = {
            0,
        };
        char* fmt_ptr = fmt_buf + fmt_buf_size;

        // required NUL terminator for sprintf formats (1)
        *--fmt_ptr = 0;

        // every sprint call must have a valid code (1)
        *--fmt_ptr = printf_spec;

        // we always pass in a width and precision, defaulting to 0 which has no effect
        // as width, and -1 which is a guaranteed "ignore" (3)
        *--fmt_ptr = '*';
        *--fmt_ptr = '.';
        *--fmt_ptr = '*';

        // leading zeroes flag (1)
        if (leading_zeroes) {
            *--fmt_ptr = '0';
        }

        // every format must start with this (1)
        *--fmt_ptr = '%';

        return std::snprintf(buf, buf_size, fmt_ptr, width, precision, value);
    }

    // std::to_chars<float> is still mostly unsupported by vendors
    template <typename Writer>
    inline void write_float(Writer& out, double value, string_view spec_string) {
        constexpr std::size_t buf_size = 1078;
        char buf[buf_size] = {
            0,
        };

        int const result = write_float_helper(buf, buf_size, value, spec_string);
        if (result > 0) {
            out.write({buf, std::size_t(result) < buf_size ? std::size_t(result) : buf_size});
        }
    }

} // namespace up::_detail
