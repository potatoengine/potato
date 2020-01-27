// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "_detail/format_traits.h"
#include "_detail/format_result.h"
#include <type_traits>
#include <potato/spud/string_view.h>

namespace up {
    class format_writer;
    class format_options;

    template <typename Writer, typename... Args> constexpr auto format_to(Writer& writer, string_view format_str, Args const& ... args) -> std::enable_if_t<_detail::is_format_writer<Writer>, up::format_result>;
    template <typename ResultT, typename... Args> constexpr ResultT format_as(string_view format_str, Args const& ... args);
    template <typename Receiver, typename... Args> constexpr format_result format_append(Receiver& receiver, string_view format_str, Args const&... args);

    template <typename Writer, typename T>
    constexpr auto format_value_to(Writer& writer, T const& value, string_view spec_string = {}) -> std::enable_if_t<_detail::is_format_writer<Writer>, up::format_result>;
}

#include "_detail/format_writer.h"
#include "_detail/append_writer.h"
#include "_detail/fixed_writer.h"
#include "_detail/format_arg.h"

namespace up {
    /// Default format helpers.
    template <typename Writer>
    void format_value(Writer& out, string_view str) {
        out.write(str);
    }
}

/// @internal
namespace up::_detail {
    UP_FORMAT_API format_result format_impl(format_writer& out, string_view format, format_arg_list args);
}

extern UP_FORMAT_API up::format_result up::_detail::format_impl(format_writer& out, string_view format, format_arg_list args);

/// Write the string format using the given parameters into a buffer.
/// @param writer The write buffer that will receive the formatted text.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Writer, typename... Args>
constexpr auto up::format_to(Writer& writer, string_view format_str, Args const& ... args) -> std::enable_if_t<_detail::is_format_writer<Writer>, up::format_result> {
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
template <typename Writer, typename T>
constexpr auto up::format_value_to(Writer& writer, T const& value, string_view spec_string) -> std::enable_if_t<_detail::is_format_writer<Writer>, up::format_result> {
    return _detail::make_format_arg(value).format_into(writer, spec_string);
}

/// Write the string format using the given parameters into a receiver.
/// @param receiver The receiver of the formatted text.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Receiver, typename... Args>
constexpr up::format_result up::format_append(Receiver& receiver, string_view format_str, Args const&... args) {
    auto writer = append_writer(receiver);
    return format_to(writer, format_str, args...);
}
