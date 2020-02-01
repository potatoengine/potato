// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/format/format.h"
#include "potato/spud/utility.h"
#include "format_spec.h"
#include <climits>
#include <limits>
#include <charconv>

namespace up::_detail {

	template <typename Writer, typename T>
    constexpr void write_integer(Writer& out, T raw, string_view spec_string) {
        constexpr auto max_hex_chars = sizeof(raw) * CHAR_BIT + 1 /*negative*/;
        constexpr auto max_dec_chars = std::numeric_limits<T>::digits10 + 2 /*overflow digit, negative*/;
        constexpr auto max_bin_chars = std::numeric_limits<T>::digits + 1 /*negative*/;
        constexpr auto max_buffer = 1/*NUL*/ + (max_hex_chars | max_dec_chars | max_bin_chars);
        constexpr auto zero_count = 8;
        constexpr auto zero_mask = zero_count - 1;
        static_assert((zero_count & zero_mask) == 0);
        constexpr char zeroes[zero_count] = { '0', '0', '0', '0', '0', '0', '0', '0' };

        bool leading_zeroes = false;
        if (auto const [success, lead_zeroes] = _detail::parse_spec(spec_string, "0"); success) {
            leading_zeroes = true;
        }

        unsigned width = 0u;
        char const* const end = spec_string.data() + spec_string.size();
        char const* next = _detail::parse_unsigned(spec_string.data(), spec_string.data() + spec_string.size(), width);
        spec_string = { next, end };

        int base = 10;
        if (auto const [success, spec] = _detail::parse_spec(spec_string, "xb"); success) {
            switch (spec) {
            case 'x': base = 16; break;
            case 'b': base = 2; break;
            default: break;
            }
        }

        char buffer[max_buffer] = { 0, };
        auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, base);

        if (result.ec == std::errc()) {
            if (leading_zeroes) {
                auto const written_width = result.ptr - buffer;
                auto remaining = width - written_width;
                while (remaining > 0) {
                    auto const to_write = remaining & zero_mask;
                    out.write({ zeroes, narrow_cast<string_view::size_type>(to_write) });
                    remaining -= to_write;
                }
            }

            out.write({buffer, result.ptr});
        }
	}

} // namespace up::_detail
