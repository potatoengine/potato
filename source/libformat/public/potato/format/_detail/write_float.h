// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "parse_format.h"
#include <cstdio>

namespace up::_detail {

    // std::to_chars<float> is still mostly unsupported by vendors
    inline void write_float(format_writer& out, double value, string_view spec_string) {
        auto const spec = parse_format_spec(spec_string);

        constexpr std::size_t fmt_buf_size = 8;
        char fmt_buf[fmt_buf_size] = {
            0,
        };
        char* fmt_ptr = fmt_buf + fmt_buf_size;

        // required NUL terminator for sprintf formats (1)
        *--fmt_ptr = 0;

        // every sprint call must have a valid code (1)
        switch (spec.code == result_code::success ? spec.options.specifier : 'f') {
        case 'a':
        case 'e':
        case 'f':
        case 'g':
            *--fmt_ptr = spec.options.specifier;
            break;
        default:
            *--fmt_ptr = 'f';
            break;
        }

        // we always pass in a width and precision, defaulting to 0 which has no effect
        // as width, and -1 which is a guaranteed "ignore" (3)
        *--fmt_ptr = '*';
        *--fmt_ptr = '.';
        *--fmt_ptr = '*';

        // leading zeroes flag (1)
        if (spec.code == result_code::success && spec.options.leading_zeroes) {
            *--fmt_ptr = '0';
        }

        // every format must start with this (1)
        *--fmt_ptr = '%';

        constexpr std::size_t buf_size = 1078;
        char buf[buf_size] = {0,};

        int const result = std::snprintf(buf, buf_size, fmt_ptr, spec.options.width, static_cast<signed char>(spec.options.precision), value);
        if (result > 0) {
            out.write({buf, std::size_t(result) < buf_size ? std::size_t(result) : buf_size});
        }
    }

} // namespace up::_detail
