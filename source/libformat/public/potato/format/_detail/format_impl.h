// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_arg.h"
#include "format_parse_nonnegative.h"
#include "format_write.h"

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
    constexpr OutputT& format_impl(OutputT& out, char const* input, char const* const end, format_args args) {
        int next = 0;
        char const* begin = input;

        while (input != end) {
            if (*input != '{') {
                ++input;
                continue;
            }

            // write out the string so far, since we don't write characters immediately
            if (input != begin) {
                format_write_n(out, begin, input - begin);
            }

            ++input; // swallow the {

            // if we hit the end of the input, we have an incomplete format, and nothing else we can do
            if (input == end) {
                return out;
            }

            // if we just have another { then take it as a literal character by starting our next begin here,
            // so it'll get written next time we write out the begin; nothing else to do for formatting here
            if (*input == '{') {
                begin = input++;
                continue;
            }

            // determine argument index
            int index = next;
            input = format_parse_nonnegative(input, end, index);

            // extract formatter specification/arguments
            if (input != end && *input == ':') {
                ++input;
            }

            // format the value
            format_parse_context ctx(input, end);
            auto const arg = args.get(index);
            arg.format_into(out, ctx);

            // consume parse specification, and any trailing }
            input = ctx.begin();
            if (input != end && *input == '}') {
                ++input;
            }

            // mark where the next text run will begin
            begin = input;

            // if we continue to receive {} then the next index will be the next one after the last one used
            next = index + 1;
        }

        // write out tail end of format string
        if (input != begin) {
            format_write_n(out, begin, input - begin);
        }

        return out;
    }

} // namespace up::_detail
