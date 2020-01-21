// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "parse_unsigned.h"

namespace up {

    constexpr UP_FORMAT_API parse_spec_result parse_format_spec(string_view spec_string) noexcept {
		using Traits = _detail::FormatTraits<char>;

        parse_spec_result result;

        char const* start = spec_string.data();
		char const* const end = start + spec_string.size();

		// flags
		while (start != end) {
			if (*start == Traits::cZero) {
                result.options.leading_zeroes = true;
			}
			else if (*start == Traits::cHash) {
                result.options.alternate_form = true;
			}
			else {
				break;
			}
			++start;
		}

		// read in width
        unsigned width = 0;
		start = _detail::parse_unsigned(start, end, width);
        result.options.width = static_cast<unsigned char>(width);

		// read in precision, if present
		if (start != end && *start == Traits::cDot) {
            unsigned precision = 0;
			start = _detail::parse_unsigned(start + 1, end, precision);
            result.options.precision = static_cast<unsigned char>(precision);
		}

		// generic code specified option allowed (mostly to set format_options on numeric formatting)
		if (start != end && Traits::sFormatSpecifiers.find(*start) != string_view::npos) {
            result.options.specifier = *start++;
		}

		return result;
	}

} // namespace up
