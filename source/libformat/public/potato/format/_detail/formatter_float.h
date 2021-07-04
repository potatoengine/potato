// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_spec.h"
#include "format_write.h"

#include <cstdio>

namespace up {
    template <>
    struct formatter<double> {
        unsigned width = 0u;
        unsigned precision = ~0u;
        bool leading_zeroes = false;
        char format_type = 'f';

        constexpr char const* parse(string_view spec) noexcept {
            char const* in = spec.begin();
            char const* const end = spec.end();

            if (in != end && *in == '0') {
                ++in;
                leading_zeroes = true;
            }

            if (in != end) {
                in = _detail::parse_unsigned(in, end, width);
            }

            if (in != end && *in == '.') {
                in = _detail::parse_unsigned(++in, end, precision);
            }

            if (in != end) {
                switch (*in) {
                    case 'a':
                    case 'e':
                    case 'f':
                    case 'g':
                        format_type = *in++;
                        break;
                    default:
                        return in;
                }
            }

            return in;
        }

        template <typename OutputT>
        void format(OutputT& output, double value) noexcept(is_format_write_noexcept<OutputT>) {
            constexpr std::size_t buf_size = 1078;
            char buf[buf_size] = {
                0,
            };

            int const result = sprintf_helper(buf, buf_size, value);
            if (result > 0) {
                size_t length = static_cast<size_t>(result);
                if (length > buf_size) {
                    length = buf_size;
                }
                format_write(output, {buf, length});
            }
        }

        int sprintf_helper(char* buf, size_t buf_size, double value) noexcept {
            constexpr std::size_t fmt_buf_size = 8;
            char fmt_buf[fmt_buf_size] = {
                0,
            };
            char* fmt_ptr = fmt_buf + fmt_buf_size;

            // required NUL terminator for sprintf formats (1)
            *--fmt_ptr = 0;

            // every sprint call must have a valid code (1)
            *--fmt_ptr = format_type;

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
    };

    template <>
    struct formatter<float> : formatter<double> {};
} // namespace up
