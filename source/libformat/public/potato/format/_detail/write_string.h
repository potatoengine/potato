// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

namespace up::_detail {

    template <typename Writer>
    constexpr void write_string(Writer& out, string_view str, format_options const&) {
        out.write(str);
	}

    template <typename Writer>
    constexpr void write_char(Writer& out, char ch, format_options const&) {
        out.write({&ch, 1});
	}

} // namespace up::_detail
