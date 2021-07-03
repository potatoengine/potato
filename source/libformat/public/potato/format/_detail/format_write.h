// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"

#include <type_traits>

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
    template <up::_detail_format::is_writeable OutputT>
    constexpr void format_write(OutputT& output, string_view text) noexcept(noexcept(output.write(text))) {
        output.write(text);
    }

    template <up::_detail_format::is_appendable OutputT>
    constexpr void format_write(OutputT& output, string_view text) noexcept(noexcept(output.append(text))) {
        output.append(text);
    }

    template <up::_detail_format::is_push_back OutputT>
    requires(!up::_detail_format::is_appendable<OutputT> && !up::_detail_format::is_writeable<OutputT>) constexpr void format_write(
        OutputT& output,
        string_view text) noexcept(noexcept(output.push_back(text.front()))) {
        for (auto const c : text) {
            output.push_back(c);
        }
    }

    template <typename OutputT>
    constexpr void format_write(OutputT& output, string_view text) noexcept {
        for (char const c : text) {
            *output++ = c;
        }
        *output = '\0';
    }

    template <typename OutputT>
    concept is_format_write_noexcept = noexcept(format_write(std::declval<OutputT>(), std::declval<up::string_view>()));
} // namespace up
