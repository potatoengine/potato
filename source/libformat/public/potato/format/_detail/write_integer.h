// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/format/format.h"
#include "parse_format.h"
#include <limits>
#include <charconv>

namespace up::_detail {

	template <typename Writer, typename T>
    constexpr void write_integer(Writer& out, T raw, string_view spec_string) {
        auto const spec = parse_format_spec(spec_string);

        if (spec.code == result_code::success && spec.options.specifier == 'x') {
            char buffer[sizeof(raw) * CHAR_BIT + 2 /*negative, nul*/] = {0,};
            auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, 16);
            if (result.ec == std::errc()) {
                out.write({buffer, result.ptr});
            }
        }
        else if (spec.code == result_code::success && spec.options.specifier == 'b') {
            char buffer[std::numeric_limits<T>::digits + 2 /*negative, nul*/] = {0,};
            auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, 2);
            if (result.ec == std::errc()) {
                out.write({buffer, result.ptr});
            }
        }
        else {
            char buffer[std::numeric_limits<T>::digits10 + 3 /*overflow digit, negative, nul*/] = {0,};
            auto const result = std::to_chars(buffer, buffer + sizeof(buffer), raw, 10);
            if (result.ec == std::errc()) {
                out.write({buffer, result.ptr});
            }
        }
	}

} // namespace up::_detail
