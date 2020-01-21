// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <type_traits>
#include <potato/spud/string_view.h>

namespace up {
    enum class result_code : unsigned int;
    enum class format_justify : unsigned char;
    enum class format_sign : unsigned char;

    class format_writer;
    class format_options;

    class parse_spec_result;

    template <typename... Args> constexpr result_code format_to(format_writer& writer, string_view format_str, Args const& ... args);
    template <typename ResultT, typename... Args> constexpr ResultT format_as(string_view format_str, Args const& ... args);
    template <typename Receiver, typename... Args> constexpr result_code format_append(Receiver& receiver, string_view format_str, Args const&... args);

    template <typename T>
    constexpr result_code format_value_to(format_writer& writer, T const& value, format_options const& options = {});

    UP_FORMAT_API parse_spec_result parse_format_spec(string_view spec_string) noexcept;
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
    UP_FORMAT_API void format_value(format_writer& out, string_view str, format_options const& options = {}) noexcept;
}

/// @internal
namespace up::_detail {
    UP_FORMAT_API result_code format_impl(format_writer& out, string_view format, format_arg_list args);
}

extern UP_FORMAT_API up::result_code up::_detail::format_impl(format_writer& out, string_view format, format_arg_list args);
extern UP_FORMAT_API up::parse_spec_result up::parse_format_spec(string_view spec_string) noexcept;

/// Write the string format using the given parameters into a buffer.
/// @param writer The write buffer that will receive the formatted text.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename... Args>
constexpr up::result_code up::format_to(format_writer& writer, string_view format_str, Args const& ... args) {
    if constexpr (sizeof...(Args) > 0) {
        const _detail::format_arg bound_args[] = { _detail::make_format_arg<_detail::formattable_t<Args>>(args)... };
        return _detail::format_impl(writer, format_str, {bound_args, sizeof...(Args)});
    }
    else {
        return _detail::format_impl(writer, format_str, {});
    }
}

/// Write the string format using the given parameters and return a string with the result.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a formatted string.
template <typename ResultT, typename... Args>
constexpr ResultT up::format_as(string_view format_str, Args const&... args) {
    ResultT result;
    format_append(result, format_str, args...);
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
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Receiver, typename... Args>
constexpr up::result_code up::format_append(Receiver& receiver, string_view format_str, Args const&... args) {
    auto writer = append_writer(receiver);
    return format_to(writer, format_str, args...);
}
