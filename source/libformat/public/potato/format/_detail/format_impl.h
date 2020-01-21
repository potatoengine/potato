// up - C++ string formatting library.
//
// This is free and unencumbered software released into the public domain.
// 
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non - commercial, and by any
// means.
// 
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// For more information, please refer to <http://unlicense.org/>
//
// Authors:
//   Sean Middleditch <sean@middleditch.us>

#if !defined(_guard_FORMATXX_DETAIL_FORMAT_IMPL_H)
#define _guard_FORMATXX_DETAIL_FORMAT_IMPL_H
#pragma once

#include "parse_unsigned.h"
#include "parse_format.h"

namespace up::_detail {

	UP_FORMAT_API result_code format_impl(format_writer& out, string_view format, format_arg_list args) {
		unsigned next_index = 0;
		result_code result = result_code::success;

		char const* begin = format.data();
		char const* const end = begin + format.size();
		char const* iter = begin;

		while (iter < end) {
			if (*iter != FormatTraits<char>::cFormatBegin) {
				++iter;
				continue;
			}
			// write out the string so far, since we don't write characters immediately
			if (iter > begin) {
				out.write({ begin, iter });
			}

			++iter; // swallow the {

			// if we hit the end of the input, we have an incomplete format, and nothing else we can do
			if (iter == end) {
				result = result_code::malformed_input;
				break;
			}

			// if we just have another { then take it as a literal character by starting our next begin here,
			// so it'll get written next time we write out the begin; nothing else to do for formatting here
			if (*iter == FormatTraits<char>::cFormatBegin) {
				begin = iter++;
				continue;
			}

			// determine which argument we're going to format
			unsigned index = 0;
			char const* const start = iter;
			iter = parse_unsigned(start, end, index);

			// if we hit the end of the string, we have an incomplete format
			if (iter == end) {
				result = result_code::malformed_input;
				break;
			}

			// if we read nothing, we have a "next index" situation (or an error)
			if (iter == start) {
				index = next_index;
			}

			format_options options;

			// if a : follows the number, we have some formatting controls
			if (*iter == FormatTraits<char>::cFormatSep) {
				++iter; // eat separator
				char const* const spec_begin = iter;

				while (iter < end && *iter != FormatTraits<char>::cFormatEnd) {
					++iter;
				}

				if (iter == end) {
					// invalid options
					result = result_code::malformed_input;
					break;
				}

                parse_spec_result const spec_result = parse_format_spec({ spec_begin, iter });
                if (spec_result.code != result_code::success) {
                    result = spec_result.code;
                    break;
                }

                options = spec_result.options;
                options.user = spec_result.unparsed;
			}

			// after the index/options, we expect an end to the format marker
			if (*iter != FormatTraits<char>::cFormatEnd) {
				// we have something besides a number, no bueno
				result = result_code::malformed_input;
				begin = iter; // make sure we're set up for the next begin, which starts at this unknown character
				continue;
			}

			result_code const arg_result = args.format_arg_into(out, index, options);
			if (arg_result != result_code::success) {
				result = arg_result;
			}

			// the iterrent text begin begins with the next character following the format directive's end
			begin = iter = iter + 1;

			// if we continue to receive {} then the next index will be the next one after the last one used
			next_index = index + 1;
		}

		// write out tail end of format string
		if (iter > begin) {
			out.write({ begin, iter });
		}

		return result;
	}

} // namespace up::_detail

#endif // _guard_FORMATXX_DETAIL_FORMAT_IMPL_H
