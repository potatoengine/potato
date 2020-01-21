// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#if !defined(_guard_FORMATXX_DETAIL_FORMAT_UTIL_H)
#define _guard_FORMATXX_DETAIL_FORMAT_UTIL_H
#pragma once

namespace up::_detail {

	inline void write_padding(format_writer& out, char pad_char, std::size_t count) {
		// FIXME: this is not even remotely the most efficient way to do this
		for (std::size_t i = 0; i != count; ++i) {
			out.write({ &pad_char, 1 });
		}
	}

    inline void write_padded_align_right(format_writer& out, string_view string, char pad_char, std::size_t count) {
		if (count > string.size()) {
			write_padding(out, pad_char, count - string.size());
		}

		out.write(string);
	}

    inline void write_padded_align_left(format_writer& out, string_view string, char pad_char, std::size_t count) {
		out.write(string);

		if (count > string.size()) {
			write_padding(out, pad_char, count - string.size());
		}
	}

    inline void write_padded_aligned(format_writer& out, string_view string, char pad_char, std::size_t count, bool align_left) {
		if (!align_left) {
			write_padded_align_right(out, string, pad_char, count);
		}
		else {
			write_padded_align_left(out, string, pad_char, count);
		}
	}

	constexpr auto trim_string(string_view string, std::size_t max_size) noexcept -> string_view {
		return string.size() < max_size ? string : string_view(string.data(), max_size);
	}

    constexpr bool string_contains(string_view haystack, char needle) noexcept {
		for (char const c : haystack) {
			if (c == needle) {
				return true;
			}
		}
		return false;
	}

} // namespace up::_detail

#endif // _guard_FORMATXX_DETAIL_FORMAT_UTIL_H
