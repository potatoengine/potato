// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"

namespace up::_detail_format {
    template <typename T>
    concept is_writeable = requires(T& o) {
        o.write(string_view{});
    };

    template <typename T>
    concept is_appendable = requires(T& o) {
        o.append(string_view{});
    };

    template <typename T>
    concept is_push_back = requires(T& o) {
        o.push_back(char{});
    };

} // namespace up::_detail_format

namespace up {
    /// Write a string to a target output iterator or writeable buffer
    template <typename OutputT>
    constexpr void format_write(OutputT& output, string_view text) {
        if constexpr (up::_detail_format::is_writeable<OutputT>) {
            output.write(text);
        }
        else if constexpr (up::_detail_format::is_appendable<OutputT>) {
            output.append(text);
        }
        else if constexpr (up::_detail_format::is_push_back<OutputT>) {
            for (auto const c : text) {
                output.push_back(c);
            }
        }
        else {
            for (char const c : text) {
                *output++ = c;
            }
            *output = '\0';
        }
    }

} // namespace up
