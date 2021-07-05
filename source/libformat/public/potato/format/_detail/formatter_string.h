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

        template <typename ContextT>
        constexpr void format(string_view value, ContextT& ctx) {
            auto const size = value.size();
            if (width < 0 || size >= static_cast<size_t>(width)) {
                up::format_write_n(ctx.out(), value.data(), size);
            }
            else {
                auto const padding = static_cast<size_t>(width) - size;
                if (left_align) {
                    up::format_write_n(ctx.out(), value.data(), size);
                    up::format_pad_n<' '>(ctx.out(), padding);
                }
                else {
                    up::format_pad_n<' '>(ctx.out(), padding);
                    up::format_write_n(ctx.out(), value.data(), size);
                }
            }
        }
    };

    template <>
    struct formatter<char const*> : formatter<string_view> {};

    template <>
    struct formatter<char> : formatter<void> {
        template <typename ContextT>
        constexpr void format(char ch, ContextT& ctx) {
            up::format_write_n(ctx.out(), &ch, 1);
        }
    };

} // namespace up
