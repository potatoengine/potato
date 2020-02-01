// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "parse_unsigned.h"

namespace up::_detail {

    struct format_width_precision {
        bool success = false;
        unsigned width = 0;
        unsigned precision = ~0u;
    };

    constexpr auto parse_width_and_precision(string_view& spec_string) noexcept -> format_width_precision {
        if (spec_string.empty()) {
            return { false };
        }

        char const* const start = spec_string.begin();
        char const* const end = spec_string.end();

        // read in width
        unsigned width = 0u;
        char const* next = _detail::parse_unsigned(start, end, width);
        bool success = next != start;

        // read in precision, if present
        unsigned precision = ~0u;
        if (next != end && *next == '.') {
            char const* const prec_string = next + 1;
            next = _detail::parse_unsigned(prec_string, end, precision);
            success = next != prec_string;
        }

        spec_string = { next, end };

        return { success, width, precision };
    }

    struct format_spec {
        bool success = false;
        char spec = '\0';
    };

    constexpr auto parse_spec(string_view& spec_string, string_view spec_options) noexcept -> format_spec {
        if (!spec_string.empty()) {
            char spec = spec_string.front();
            if (spec_options.find(spec) != string_view::npos) {
                spec_string.pop_front();
                return { true, spec };
            }
        }
        return { false };
    }

} // namespace up::_detail
