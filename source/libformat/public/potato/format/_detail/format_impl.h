// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "parse_unsigned.h"

namespace up::_detail {

	UP_FORMAT_API format_result format_impl(format_writer& out, string_view format, format_arg_list args) {
		unsigned next_index = 0;
		format_result result = format_result::success;

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
				out.write({ begin, iter });
			}

			++iter; // swallow the {

			// if we hit the end of the input, we have an incomplete format, and nothing else we can do
			if (iter == end) {
				result = format_result::malformed_input;
				break;
			}

			// if we just have another { then take it as a literal character by starting our next begin here,
			// so it'll get written next time we write out the begin; nothing else to do for formatting here
			if (*iter == '{') {
				begin = iter++;
				continue;
			}

			// determine which argument we're going to format
			unsigned index = next_index;
			char const* const start = iter;
			iter = parse_unsigned(start, end, index);

			// if we hit the end of the string, we have an incomplete format
			if (iter == end) {
				result = format_result::malformed_input;
				break;
			}

			string_view spec_string;

			// if a : follows the number, we have some formatting controls
			if (*iter == ':') {
				++iter; // eat separator
				char const* const spec_begin = iter;

				while (iter != end && *iter != '}') {
					++iter;
				}

				if (iter == end) {
					// invalid options
					result = format_result::malformed_input;
					break;
				}

                spec_string = { spec_begin, iter };
			}

			// after the index/options, we expect an end to the format marker
			if (*iter != '}') {
				// we have something besides a number, no bueno
				result = format_result::malformed_input;
                break;
			}

			format_result const arg_result = args.format_arg_into(out, index, spec_string);
			if (arg_result != format_result::success) {
				result = arg_result;
			}

			// the remaining text begins with the next character following the format directive's end
			begin = iter = iter + 1;

			// if we continue to receive {} then the next index will be the next one after the last one used
			next_index = index + 1;
		}

		// write out tail end of format string
		if (iter != begin) {
			out.write({ begin, iter });
		}

		return result;
	}

} // namespace up::_detail
