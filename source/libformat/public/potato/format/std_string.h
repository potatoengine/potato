// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format.h"
#include <string>
#include <string_view>

namespace up {
    template <typename Writer, typename StringCharT, typename TraitsT, typename AllocatorT>
    constexpr void format_value(Writer& out, std::basic_string<StringCharT, TraitsT, AllocatorT> const& string) {
        format_value(out, string_view(string.data(), string.size()));
    }

    template <typename Writer, typename StringCharT, typename TraitsT>
    constexpr void format_value(Writer& out, std::basic_string_view<StringCharT, TraitsT> const& string) {
        format_value(out, string_view(string.data(), string.size()));
    }
} // namespace up
