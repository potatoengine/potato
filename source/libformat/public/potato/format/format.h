// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include "_detail/format_traits.h"
#include "_detail/format_result.h"
#include "_detail/append_writer.h"
#include "_detail/fixed_writer.h"
#include "_detail/format_arg.h"
#include "_detail/format_arg_impl.h"
#include "_detail/format_impl.h"
#include <potato/spud/string_view.h>
#include <type_traits>

namespace up {
    template <typename Writer, typename... Args> constexpr auto format_to(Writer& writer, string_view format_str, Args const& ... args) -> std::enable_if_t<is_format_writer<Writer> && (is_formattable<Args> && ...), up::format_result>;
    template <typename ResultT, typename... Args> constexpr auto format_as(string_view format_str, Args const& ... args) -> std::enable_if_t<(is_formattable<Args> && ...), ResultT>;
    template <typename Receiver, typename... Args> constexpr auto format_append(Receiver& receiver, string_view format_str, Args const&... args) -> std::enable_if_t<(is_formattable<Args> && ...), format_result>;

    template <typename Writer, typename T>
    constexpr auto format_value_to(Writer& writer, T const& value, string_view spec_string = {}) -> std::enable_if_t<is_format_writer<Writer> && is_formattable<T>, up::format_result>;
}

namespace up {
    /// Default format helpers.
    template <typename Writer>
    constexpr void format_value(Writer& out, string_view str) noexcept(noexcept(out.write(str))) {
        out.write(str);
    }
}

/// Write the string format using the given parameters into a buffer.
/// @param writer The write buffer that will receive the formatted text.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Writer, typename... Args>
constexpr auto up::format_to(Writer& writer, string_view format_str, Args const& ... args) -> std::enable_if_t<is_format_writer<Writer> && (is_formattable<Args> && ...), up::format_result> {
    if constexpr (sizeof...(Args) > 0) {
        const _detail::format_arg bound_args[] = { _detail::make_format_arg<Writer, _detail::formattable_t<Args>>(args)... };
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
constexpr auto up::format_as(string_view format_str, Args const&... args) -> std::enable_if_t<(is_formattable<Args> && ...), ResultT> {
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
constexpr auto up::format_value_to(Writer& writer, T const& value, string_view spec_string) -> std::enable_if_t<is_format_writer<Writer> && is_formattable<T>, up::format_result> {
    return _detail::make_format_arg<Writer>(value).format_into(writer, spec_string);
}

/// Write the string format using the given parameters into a receiver.
/// @param receiver The receiver of the formatted text.
/// @param format_str The primary text and formatting controls to be written.
/// @param args The arguments used by the formatting string.
/// @returns a result code indicating any errors.
template <typename Receiver, typename... Args>
constexpr auto up::format_append(Receiver& receiver, string_view format_str, Args const&... args) -> std::enable_if_t<(is_formattable<Args> && ...), format_result> {
    auto writer = append_writer(receiver);
    return format_to(writer, format_str, args...);
}
