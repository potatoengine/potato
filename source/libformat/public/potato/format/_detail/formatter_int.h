// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_write.h"
#include "formatter.h"

#include "potato/format/format.h"
#include "potato/spud/ascii.h"
#include "potato/spud/platform.h"
#include "potato/spud/utility.h"

#include <charconv>
#include <climits>
#include <limits>

namespace up::_detail_format {
    template <typename IntT>
    struct int_formatter {
        int width = -1;
        int base = 10;
        bool leading_zeroes = false;
        bool uppercase = false;

        constexpr char const* parse(format_parse_context& ctx) noexcept {
            char const* in = ctx.begin();
            char const* const end = ctx.end();

            if (in != end && *in == '0') {
                ++in;
                leading_zeroes = true;
            }

            if (in != end) {
                in = format_parse_nonnegative(in, end, width);
            }

            if (in != end) {
                switch (*in) {
                    case 'x':
                        base = 16;
                        break;
                    case 'X':
                        base = 16;
                        uppercase = true;
                        break;
                    case 'b':
                        base = 2;
                        break;
                    case 'd':
                    case 'i':
                    case 'u':
                        break;
                    default:
                        return in;
                }
                ++in;
            }

            return in;
        }

        void format(IntT value, format_context& ctx) {
            constexpr auto buffer_size = sizeof(value) * CHAR_BIT + 1 /*negative*/;
            char buffer[buffer_size] = {
                0,
            };

            auto const result = std::to_chars(buffer, buffer + buffer_size, value, base);
            if (result.ec != std::errc()) {
                return;
            }

            auto const length = result.ptr - buffer;

            if (uppercase) {
                for (char *c = buffer, *const end = buffer + length; c != end; ++c) {
                    if (*c >= 'a' && *c <= 'z') {
                        *c = *c - 'a' + 'A';
                    }
                }
            }

            if (width >= 0 && static_cast<decltype(length)>(width) > length) {
                auto const padding = static_cast<decltype(length)>(width) - length;
                if (leading_zeroes) {
                    format_pad_n<'0'>(ctx.out(), padding);
                }
                else {
                    format_pad_n<' '>(ctx.out(), padding);
                }
            }

            format_write_n(ctx.out(), buffer, result.ptr - buffer);
        }
    };
} // namespace up::_detail_format

namespace up {
    template <>
    struct formatter<int> : _detail_format::int_formatter<int> {};

    template <>
    struct formatter<unsigned> : _detail_format::int_formatter<unsigned> {};

    template <>
    struct formatter<long long> : _detail_format::int_formatter<long long> {};

    template <>
    struct formatter<unsigned long long> : _detail_format::int_formatter<unsigned long long> {};
} // namespace up
