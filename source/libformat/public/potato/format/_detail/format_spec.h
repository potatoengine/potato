// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "parse_unsigned.h"

namespace up::_detail {
    struct format_spec {
        bool success = false;
        char spec = '\0';
    };

    constexpr auto parse_spec(string_view& spec_string, string_view spec_options) noexcept -> format_spec {
        if (!spec_string.empty()) {
            char spec = spec_string.front();
            if (spec_options.find(spec) != string_view::npos) {
                spec_string.pop_front();
                return {true, spec};
            }
        }
        return {false};
    }

} // namespace up::_detail
