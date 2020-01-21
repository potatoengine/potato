// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

namespace up::_detail {

	static constexpr char const* parse_unsigned(char const* start, char const* end, unsigned& result) noexcept {
		result = 0;
            while (start != end && *start >= char('0') && *start <= char('9')) {
			result *= 10;
			result += *start - char('0');
			++start;
		}
		return start;
	}

} // namespace up::_detail
