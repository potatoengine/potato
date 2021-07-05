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
    template <typename OutputT>
    constexpr void format_write_n(OutputT& output, char const* str, size_t n) {
        if constexpr (_detail_format::is_appendable<OutputT>) {
            output.append(str, n);
        }
        else if constexpr (_detail_format::is_push_back<OutputT>) {
            for (; n != 0; ++str, --n) {
                output.push_back(*str);
            }
        }
        else {
            for (; n != 0; ++str, ++output, --n) {
                *output = *str;
            }
        }
    }

    template <char PadChar, typename OutputT>
    constexpr void format_pad_n(OutputT& out, size_t width) {
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
} // namespace up
