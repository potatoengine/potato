// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_spec.h"

#include "potato/format/format.h"
#include "potato/spud/utility.h"

#include <charconv>
#include <climits>
#include <limits>

namespace up::_detail {

    template <char PadChar, typename Writer> constexpr void write_padding(Writer& out, size_t width) noexcept {
        constexpr auto pad_run_count = 8;
        constexpr auto pad_run_mask = pad_run_count - 1;
        static_assert((pad_run_count & pad_run_mask) == 0);
        constexpr char padding[pad_run_count] = {PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar};

        while (width > 0) {
            auto const to_write = width & pad_run_mask;
            out.write({padding, narrow_cast<string_view::size_type>(to_write)});
            width -= to_write;
        }
    }

    struct integer_spec {
        unsigned width = 0u;
        int base = 10;
        bool leading_zeroes = false;
    };

    constexpr integer_spec parse_integer_spec(string_view spec_string) noexcept {
        integer_spec spec;

        if (auto const [success, lead_zeroes] = _detail::parse_spec(spec_string, "0"); success) {
            spec.leading_zeroes = true;
        }

        char const* const end = spec_string.data() + spec_string.size();
        char const* next = _detail::parse_unsigned(spec_string.data(), spec_string.data() + spec_string.size(), spec.width);
        spec_string = {next, end};

        if (auto const [success, printf_spec] = _detail::parse_spec(spec_string, "xb"); success) {
            switch (printf_spec) {
            case 'x': spec.base = 16; break;
            case 'b': spec.base = 2; break;
            default: break;
            }
        }

        return spec;
    }

    template <typename Writer, typename T> constexpr void write_integer(Writer& out, T raw, string_view spec_string) {
        constexpr auto max_hex_chars = sizeof(raw) * CHAR_BIT + 1 /*negative*/;
        constexpr auto max_dec_chars = std::numeric_limits<T>::digits10 + 2 /*overflow digit, negative*/;
        constexpr auto max_bin_chars = std::numeric_limits<T>::digits + 1 /*negative*/;
        constexpr auto max_buffer = 1 /*NUL*/ + (max_hex_chars | max_dec_chars | max_bin_chars);

        auto const spec = parse_integer_spec(spec_string);

        char buffer[max_buffer] = {
            0,
        };
        auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, spec.base);

        if (result.ec == std::errc()) {
            if (spec.width > 0u) {
                auto const written_width = result.ptr - buffer;
                auto const required_padding = spec.width - written_width;
                if (spec.leading_zeroes) {
                    write_padding<'0'>(out, required_padding);
                }
                else {
                    write_padding<' '>(out, required_padding);
                }
            }

            out.write({buffer, result.ptr});
        }
    }

} // namespace up::_detail
