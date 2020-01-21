// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#if !defined(_guard_FORMATXX_STD_STRING_H)
#define _guard_FORMATXX_STD_STRING_H
#pragma once

#include "format.h"
#include <string>
#include <string_view>

namespace up {
    template <typename StringCharT, typename TraitsT, typename AllocatorT>
	constexpr void format_value(format_writer& out, std::basic_string<StringCharT, TraitsT, AllocatorT> const& string, format_options const& options) {
		format_value(out, string_view(string.data(), string.size()), options);
	}

    template <typename StringCharT, typename TraitsT>
    constexpr void format_value(format_writer& out, std::basic_string_view<StringCharT, TraitsT> const& string, format_options const& options) {
        format_value(out, string_view(string.data(), string.size()), options);
    }
} // namespace formatxx

#endif // !defined(_guard_FORMATXX_STD_STRING_H)
