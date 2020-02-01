// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/format/format.h"
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

        char buffer[max_buffer] = { 0, };
        int base = 10;

        if (auto const [success, spec] = _detail::parse_spec(spec_string, "xb"); success) {
            switch (spec) {
            case 'x': base = 16; break;
            case 'b': base = 2; break;
            default: break;
            }
        }

        auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, base);

        if (result.ec == std::errc()) {
            out.write({buffer, result.ptr});
        }
	}

} // namespace up::_detail
