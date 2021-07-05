// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_arg.h"
#include "format_parse_nonnegative.h"
#include "format_write.h"

#include "potato/spud/string_view.h"

#include <initializer_list>

namespace up::_detail {
    template <typename OutputT>
    struct format_impl_context {
        OutputT& out;
        char const* input = nullptr;
        char const* end = nullptr;
        format_args args;
        int next = 0;
    };

    template <typename OutputT>
    constexpr OutputT& format_impl(format_impl_context<OutputT>&& ctx) {
        char const* begin = ctx.input;

        while (ctx.input != ctx.end) {
            if (*ctx.input != '{') {
                ++ctx.input;
                continue;
            }

            // write out the string so far, since we don't write characters immediately
            if (ctx.input != begin) {
                format_write_n(ctx.out, begin, ctx.input - begin);
            }

            ++ctx.input; // swallow the {

            // if we hit the end of the input, we have an incomplete format, and nothing else we can do
            if (ctx.input == ctx.end) {
                return ctx.out;
            }

            // if we just have another { then take it as a literal character by starting our next begin here,
            // so it'll get written next time we write out the begin; nothing else to do for formatting here
            if (*ctx.input == '{') {
                begin = ctx.input++;
                continue;
            }

            // determine argument index
            int index = ctx.next;
            ctx.input = format_parse_nonnegative(ctx.input, ctx.end, index);

            // extract formatter specification/arguments
            if (ctx.input != ctx.end && *ctx.input == ':') {
                ++ctx.input;
            }

            // format the value
            format_parse_context pctx(ctx.input, ctx.end);
            auto const arg = ctx.args.get(index);
            arg.format_into(ctx.out, pctx);

            // consume parse specification, and any trailing }
            ctx.input = pctx.begin();
            if (ctx.input != ctx.end && *ctx.input == '}') {
                ++ctx.input;
            }

            // mark where the next text run will begin
            begin = ctx.input;

            // if we continue to receive {} then the next index will be the next one after the last one used
            ctx.next = index + 1;
        }

        // write out tail end of format string
        if (ctx.input != begin) {
            format_write_n(ctx.out, begin, ctx.input - begin);
        }

        return ctx.out;
    }

} // namespace up::_detail
