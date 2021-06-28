// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_write.h"

namespace up::_detail {

    template <typename OutputT>
    constexpr void write_string(OutputT& out, string_view str, string_view) {
        up::format_write(out, str);
    }

    template <typename OutputT>
    constexpr void write_char(OutputT& out, char ch, string_view) {
        up::format_write(out, {&ch, 1});
    }

} // namespace up::_detail
