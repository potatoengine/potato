// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up::_detail {

    template <typename Writer>
    constexpr void write_string(Writer& out, string_view str, string_view) {
        out.write(str);
    }

    template <typename Writer>
    constexpr void write_char(Writer& out, char ch, string_view) {
        out.write({&ch, 1});
    }

} // namespace up::_detail
