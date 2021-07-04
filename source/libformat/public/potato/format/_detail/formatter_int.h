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

namespace up::_detail {
    template <char PadChar, typename OutputT>
    constexpr void write_padding(OutputT& out, size_t width) noexcept(is_format_write_noexcept<OutputT>) {
        constexpr auto pad_run_count = 8;
        constexpr char padding[pad_run_count] =
            {PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar};

        while (width > pad_run_count) {
            format_write(out, {padding, pad_run_count});
            width -= pad_run_count;
        }

        if (width > 0) {
            format_write(out, {padding, width});
        }
    }

    template <typename IntT>
    struct int_formatter {
        unsigned width = 0u;
        int base = 10;
        bool leading_zeroes = false;
        bool uppercase = false;

        constexpr char const* parse(string_view spec) noexcept {
            char const* in = spec.begin();
            char const* const end = spec.end();

            if (in == end) {
                return in;
            }

            if (in != end && *in == '\0') {
                ++in;
                leading_zeroes = true;
            }

            if (in != end) {
                in = parse_unsigned(in, end, width);
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

        // GCC 11.1 seems to spuriously emit this error in the uppercase loop
#if UP_COMPILER_GCC
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

        template <typename OutputT>
        void format(OutputT& output, IntT value) noexcept(is_format_write_noexcept<OutputT>) {
            constexpr auto max_hex_chars = sizeof(value) * CHAR_BIT + 1 /*negative*/;
            constexpr auto max_dec_chars = std::numeric_limits<IntT>::digits10 + 2 /*overflow digit, negative*/;
            constexpr auto max_bin_chars = std::numeric_limits<IntT>::digits + 1 /*negative*/;
            constexpr auto max_buffer = 1 /*NUL*/ + (max_hex_chars | max_dec_chars | max_bin_chars);

            char buffer[max_buffer] = {
                0,
            };
            auto const result = std::to_chars(buffer, buffer + sizeof(buffer), value, base);
            if (result.ec != std::errc()) {
                return;
            }

            if (uppercase) {
                for (char* c = buffer; c != result.ptr; ++c) {
                    *c = ascii::toUppercase(*c);
                }
            }

            if (width > 0U) {
                auto const written_width = result.ptr - buffer;
                auto const required_padding = width > written_width ? width - written_width : 0;
                if (leading_zeroes) {
                    write_padding<'0'>(output, required_padding);
                }
                else {
                    write_padding<' '>(output, required_padding);
                }
            }

            format_write(output, {buffer, result.ptr});
        }

#if UP_COMPILER_GCC
#    pragma GCC diagnostic pop
#endif
    };
} // namespace up::_detail

namespace up {
    template <>
    struct formatter<int> : _detail::int_formatter<int> {};

    template <>
    struct formatter<unsigned> : _detail::int_formatter<unsigned> {};

    template <>
    struct formatter<long long> : _detail::int_formatter<long long> {};

    template <>
    struct formatter<unsigned long long> : _detail::int_formatter<unsigned long long> {};
} // namespace up
