// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <type_traits>

namespace up::_detail_format {
    template <typename T>
    concept is_appendable = requires(T& o, char const* c, size_t n) {
        o.append(c, n);
    };

    template <typename T>
    concept is_push_back = requires(T& o, char c) {
        o.push_back(c);
    };
} // namespace up::_detail_format

namespace up {
    /// Write a string to a target output iterator or writeable buffer
    template <up::_detail_format::is_appendable OutputT>
    constexpr void format_write_n(OutputT& output, char const* str, size_t n) noexcept(
        noexcept(output.append(str, n))) {
        output.append(str, n);
    }

    template <up::_detail_format::is_push_back OutputT>
    requires(!up::_detail_format::is_appendable<OutputT>) constexpr void format_write_n(
        OutputT& output,
        char const* str,
        size_t n) noexcept(noexcept(output.push_back(*str))) {
        for (; n != 0; ++str, --n) {
            output.push_back(*str);
        }
    }

    template <typename OutputT>
    constexpr void format_write_n(OutputT& output, char const* str, size_t n) noexcept {
        for (; n != 0; ++str, ++output, --n) {
            *output = *str;
        }
    }
    template <char PadChar, typename OutputT>
    constexpr void format_pad_n(OutputT& out, size_t width) noexcept(is_format_write_noexcept<OutputT>) {
        constexpr size_t pad_run_count = 8;
        constexpr char padding[pad_run_count] =
            {PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar, PadChar};

        while (width > pad_run_count) {
            format_write_n(out, padding, pad_run_count);
            width -= pad_run_count;
        }

        if (width > 0) {
            format_write_n(out, padding, width);
        }
    }

    template <typename OutputT>
    concept is_format_write_noexcept = noexcept(format_write(std::declval<OutputT>(), "", size_t{}));
} // namespace up
