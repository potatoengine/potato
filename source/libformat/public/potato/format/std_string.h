// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format.h"

#include <string>
#include <string_view>

namespace up {
    template <typename StringCharT, typename TraitsT, typename AllocatorT>
    struct formatter<std::basic_string<StringCharT, TraitsT, AllocatorT>> : formatter<void> {
        template <typename OutputT>
        void format(OutputT& output, std::basic_string<StringCharT, TraitsT, AllocatorT> const& value) noexcept(
            is_format_write_noexcept<OutputT>) {
            format_write(output, {value.data(), value.size()});
        }
    };

    template <typename StringCharT, typename TraitsT>
    struct formatter<std::basic_string_view<StringCharT, TraitsT>> : formatter<void> {
        template <typename OutputT>
        void format(OutputT& output, std::basic_string_view<StringCharT, TraitsT> const& value) noexcept(
            is_format_write_noexcept<OutputT>) {
            format_write(output, {value.data(), value.size()});
        }
    };
} // namespace up
