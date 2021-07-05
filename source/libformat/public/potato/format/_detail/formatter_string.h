// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_parse_nonnegative.h"
#include "format_write.h"
#include "formatter.h"

namespace up {
    template <>
    struct formatter<string_view> {
        int width = -1;
        bool left_align = false;

        constexpr char const* parse(format_parse_context& ctx) noexcept {
            char const* in = ctx.begin();
            char const* const end = ctx.end();

            if (in == end) {
                return in;
            }

            if (*in == '-') {
                ++in;
                left_align = true;
            }

            in = format_parse_nonnegative(in, end, width);

            return in;
        }

        template <typename OutputT>
        constexpr void format(OutputT& output, string_view value) noexcept(is_format_write_noexcept<OutputT>) {
            auto const size = value.size();
            if (width < 0 || size >= static_cast<size_t>(width)) {
                up::format_write_n(output, value.data(), size);
            }
            else {
                auto const padding = static_cast<size_t>(width) - size;
                if (left_align) {
                    up::format_write_n(output, value.data(), size);
                    up::format_pad_n<' '>(output, padding);
                }
                else {
                    up::format_pad_n<' '>(output, padding);
                    up::format_write_n(output, value.data(), size);
                }
            }
        }
    };

    template <>
    struct formatter<char const*> : formatter<string_view> {};

    template <>
    struct formatter<char> : formatter<void> {
        template <typename OutputT>
        constexpr void format(OutputT& out, char ch) noexcept(is_format_write_noexcept<OutputT>) {
            up::format_write_n(out, &ch, 1);
        }
    };

} // namespace up
