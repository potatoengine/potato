// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <potato/spud/string_view.h>
#include "format_result.h"
#include "parse_unsigned.h"
#include <initializer_list>

namespace up::_detail {

    struct format_impl_inner_result {
        format_result result;
        unsigned index = 0;
        string_view spec_string;
    };

    constexpr format_impl_inner_result format_impl_inner(char const*& begin, char const* const end, char const*& iter, unsigned next_index) noexcept {
        // determine which argument we're going to format
        unsigned index = next_index;
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

            if (iter == end) {
                // invalid options
                return {format_result::malformed_input};
            }

            spec_string = {spec_begin, iter};
        }

        // after the index/options, we expect an end to the format marker
        if (*iter != '}') {
            // we have something besides a number, no bueno
            return {format_result::malformed_input};
        }

        return {format_result::success, index, spec_string};
    }

    template <typename Writer>
    constexpr format_result format_impl(Writer& out, string_view format, std::initializer_list<format_arg> args) {
        unsigned next_index = 0;

        char const* begin = format.data();
        char const* const end = begin + format.size();
        char const* iter = begin;

        while (iter != end) {
            if (*iter != '{') {
                ++iter;
                continue;
            }

            // write out the string so far, since we don't write characters immediately
            if (iter != begin) {
                out.write({begin, iter});
            }

            ++iter; // swallow the {

            // if we hit the end of the input, we have an incomplete format, and nothing else we can do
            if (iter == end) {
                return format_result::malformed_input;
            }

            // if we just have another { then take it as a literal character by starting our next begin here,
            // so it'll get written next time we write out the begin; nothing else to do for formatting here
            if (*iter == '{') {
                begin = iter++;
                continue;
            }

            auto const [result, index, spec_string] = format_impl_inner(begin, end, iter, next_index);
            if (result != format_result::success) {
                return result;
            }

            if (index >= args.size()) {
                return format_result::out_of_range;
            }

            format_arg const& arg = *(args.begin() + index);
            format_result const arg_result = arg.format_into(out, spec_string);
            if (arg_result != format_result::success) {
                return arg_result;
            }

            // the remaining text begins with the next character following the format directive's end
            begin = iter = iter + 1;

            // if we continue to receive {} then the next index will be the next one after the last one used
            next_index = index + 1;
        }

        // write out tail end of format string
        if (iter != begin) {
            out.write({begin, iter});
        }

        return format_result::success;
    }

} // namespace up::_detail
