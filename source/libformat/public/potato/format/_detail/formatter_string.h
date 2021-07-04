// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_write.h"
#include "formatter.h"

namespace up {
    template <>
    struct formatter<string_view> {
        template <typename OutputT>
        constexpr void format(OutputT& output, string_view value) noexcept(is_format_write_noexcept<OutputT>) {
            up::format_write(output, value);
        }
    };

    template <>
    struct formatter<char const*> {
        template <typename OutputT>
        constexpr void format(OutputT& output, const char* const value) noexcept(is_format_write_noexcept<OutputT>) {
            up::format_write(output, value);
        }
    };

    template <>
    struct formatter<char> {
        template <typename OutputT>
        constexpr void format(OutputT& out, char ch) noexcept(is_format_write_noexcept<OutputT>) {
            up::format_write(out, {&ch, 1});
        }
    };

} // namespace up
