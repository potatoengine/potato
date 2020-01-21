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

#if !defined(_guard_FORMATXX_H)
#define _guard_FORMATXX_H
#pragma once

#include <type_traits>
#include <potato/spud/string_view.h>

#if !defined(FORMATXX_API)
#	if defined(_WIN32)
#		define FORMATXX_API __stdcall
#	else
#		define FORMATXX_API
#	endif
#endif

#if defined(_WIN32) && !defined(FORMATXX_PUBLIC)
#	if defined(FORMATXX_EXPORT)
#		define FORMATXX_PUBLIC __declspec(dllexport)
#	else
#		define FORMATXX_PUBLIC
#	endif
#elif __GNUC__ >= 4 && !defined(FORMATXX_PUBLIC)
#	if defined(FORMATXX_EXPORT)
#		define FORMATXX_PUBLIC __attribute__((visibility("default")))
#	else
#		define FORMATXX_PUBLIC
#	endif
#endif

namespace up {
    enum class result_code : unsigned int;
    enum class format_justify : unsigned char;
    enum class format_sign : unsigned char;

    class format_writer;
    class format_options;

    class parse_spec_result;

    template <typename FormatT, typename... Args> constexpr result_code format_to(format_writer& writer, FormatT const& format, Args const& ... args);

    template <typename ResultT, typename FormatT, typename... Args> constexpr ResultT format_as(FormatT const& format, Args const& ... args);

    template <typename Receiver, typename FormatT, typename... Args> constexpr result_code format_append(Receiver& receiver, FormatT const& format, Args const&... args);

    template <typename T>
    constexpr result_code format_value_to(format_writer& writer, T const& value, format_options const& options = {});

    FORMATXX_PUBLIC parse_spec_result FORMATXX_API parse_format_spec(string_view spec_string) noexcept;
}

enum class up::result_code : unsigned int {
    success,
    out_of_range,
    malformed_input,
    out_of_space,
};

enum class up::format_justify : unsigned char {
    right,
    left,
    center
};

enum class up::format_sign : unsigned char {
    negative,
    always,
    space
};

#include "_detail/format_writer.h"
#include "_detail/append_writer.h"
#include "_detail/format_arg.h"

/// Extra formatting specifications.
class up::format_options {
public:
    constexpr format_options() noexcept : alternate_form(false), leading_zeroes(false) {}

    string_view user;
    unsigned width = 0;
    unsigned precision = ~0u;
    char specifier = 0;
    format_justify justify = format_justify::right;
    format_sign sign = format_sign::negative;
    bool alternate_form : 1;
    bool leading_zeroes : 1;
};

/// Result from parse_format_spec.
class up::parse_spec_result {
public:
    result_code code = result_code::success;
    format_options options;
    string_view unparsed;
};

namespace up {
    /// Default format helpers.
    FORMATXX_PUBLIC void FORMATXX_API format_value(format_writer& out, string_view str, format_options const& options = {}) noexcept;
}

/// @internal
namespace up::_detail {
    FORMATXX_PUBLIC result_code FORMATXX_API format_impl(format_writer& out, string_view format, format_arg_list args);
}

extern FORMATXX_PUBLIC up::result_code FORMATXX_API up::_detail::format_impl(format_writer& out, string_view format, format_arg_list args);
extern FORMATXX_PUBLIC up::parse_spec_result FORMATXX_API up::parse_format_spec(string_view spec_string) noexcept;

/// Write the string format using the given parameters into a buffer.
/// @param writer The write buffer that will receive the formatted text.
/// @param format The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename FormatT, typename... Args>
constexpr up::result_code up::format_to(format_writer& writer, FormatT const& format, Args const& ... args) {
    if constexpr (sizeof...(Args) > 0) {
        const _detail::format_arg bound_args[] = { _detail::make_format_arg<_detail::formattable_t<Args>>(args)... };
        return _detail::format_impl(writer, string_view(format), {bound_args, sizeof...(Args)});
    }
    else {
        return _detail::format_impl(writer, string_view(format), {});
    }
}

/// Write the string format using the given parameters and return a string with the result.
/// @param format The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a formatted string.
template <typename ResultT, typename FormatT, typename... Args>
constexpr ResultT up::format_as(FormatT const& format, Args const& ... args) {
    ResultT result;
    append_writer<ResultT> writer(result);
    format_to(writer, format, args...);
    return result;
}

/// Format a value into a buffer using the given options.
/// @param writer The write buffer that will receive the formatted text.
/// @param value The value to format.
/// @param options The format control options.
/// @returns a result code indicating any errors.
template <typename T>
constexpr up::result_code up::format_value_to(format_writer& writer, T const& value, format_options const& options) {
    return _detail::make_format_arg(value).format_into(writer, options);
}

/// Write the string format using the given parameters into a receiver.
/// @param receiver The receiver of the formatted text.
/// @param format The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Receiver, typename FormatT, typename... Args>
constexpr up::result_code up::format_append(Receiver& receiver, FormatT const& format, Args const&... args) {
    using Writer = append_writer<Receiver>;
    auto writer = Writer(receiver);
    return format_to(writer, string_view(format), args...);
}

#endif // !defined(_guard_FORMATXX_H)
