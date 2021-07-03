// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_arg.h"
#include "format_result.h"
#include "format_write.h"
#include "parse_unsigned.h"

#include "potato/spud/string_view.h"

#include <initializer_list>

namespace up::_detail {
    template <typename OutputT>
    struct format_context {
        OutputT& out;
        char const* input = nullptr;
        char const* end = nullptr;
        format_args args;
        int next = 0;
    };

    struct format_impl_inner_result {
        format_result result;
        int index = 0;
        string_view spec_string;
    };

    constexpr format_impl_inner_result format_impl_inner(
        char const*& iter,
        char const* const end,
        int next_index) noexcept {
        // determine which argument we're going to format
        int index = next_index;
        char const* const start = iter;
        iter = parse_unsigned(start, end, index);

        // if we hit the end of the string, we have an incomplete format
        if (iter == end) {
            return {format_result::malformed_input};
        }

        string_view spec_string = {};

        // if a : follows the number, we have some formatting controls
        if (*iter == ':') {
            ++iter; // eat separator
            char const* const spec_begin = iter;

            while (iter != end && *iter != '}') {
                ++iter;
            }

            spec_string = {spec_begin, iter};
        }

        // after the index/options, we expect an end to the format marker
        if (iter == end || *iter != '}') {
            return {format_result::malformed_input};
        }

        return {format_result::success, index, spec_string};
    }

    template <typename OutputT>
    constexpr format_result format_impl(format_context<OutputT>&& ctx) {
        char const* begin = ctx.input;

        while (ctx.input != ctx.end) {
            if (*ctx.input != '{') {
                ++ctx.input;
                continue;
            }

            // write out the string so far, since we don't write characters immediately
            if (ctx.input != begin) {
                format_write(ctx.out, {begin, ctx.input});
            }

            ++ctx.input; // swallow the {

            // if we hit the end of the input, we have an incomplete format, and nothing else we can do
            if (ctx.input == ctx.end) {
                return format_result::malformed_input;
            }

            // if we just have another { then take it as a literal character by starting our next begin here,
            // so it'll get written next time we write out the begin; nothing else to do for formatting here
            if (*ctx.input == '{') {
                begin = ctx.input++;
                continue;
            }

            auto const [result, index, spec_string] = format_impl_inner(ctx.input, ctx.end, ctx.next);
            if (result != format_result::success) {
                return result;
            }

            if (format_result const arg_result = ctx.args.format_into(ctx.out, index, spec_string);
                arg_result != format_result::success) {
                return arg_result;
            }

            // the remaining text begins with the next character following the format directive's end
            begin = ++ctx.input;

            // if we continue to receive {} then the next index will be the next one after the last one used
            ctx.next = index + 1;
        }

        // write out tail end of format string
        if (ctx.input != begin) {
            format_write(ctx.out, {begin, ctx.input});
        }

        return format_result::success;
    }

} // namespace up::_detail
