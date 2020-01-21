// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#if !defined(_guard_FORMATXX_DETAIL_WRITE_STRING_H)
#define _guard_FORMATXX_DETAIL_WRITE_STRING_H
#pragma once

#include "format_util.h"

namespace up::_detail {

    constexpr void write_string(format_writer& out, string_view str, format_options const& options) {
		if (options.precision != ~0u) {
			str = trim_string(str, options.precision);
		}

		if (options.justify == format_justify::right) {
			write_padded_align_right(out, str, FormatTraits<char>::cSpace, options.width);
		}
        else {
			write_padded_align_left(out, str, FormatTraits<char>::cSpace, options.width);
		}
	}

    constexpr void write_char(format_writer& out, char ch, format_options const& options) {
		write_string(out, { &ch, 1 }, options);
	}

} // namespace up::_detail

#endif // _guard_FORMATXX_DETAIL_WRITE_STRING_H
