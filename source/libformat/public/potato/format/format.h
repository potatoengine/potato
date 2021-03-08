// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_detail/append_writer.h"
#include "_detail/fixed_writer.h"
#include "_detail/format_arg.h"
#include "_detail/format_arg_impl.h"
#include "_detail/format_impl.h"
#include "_detail/format_result.h"
#include "_detail/format_traits.h"

#include "potato/spud/span.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <type_traits>

namespace up {
    /// Default format helpers.
    template <format_writable Writer>
    constexpr void format_value(Writer& out, string_view str) noexcept(noexcept(out.write(str))) {
        out.write(str);
    }

    /// Write the string format using the given parameters into a buffer.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <format_writable Writer, formattable... Args>
    constexpr auto format_to(Writer& writer, string_view format_str, Args const&... args) -> format_result {
        // The argument list _must_ be a temporary in the parameter list, as conversions may be involved in
        // formattable_t constructor; trying to store this in a local array or variable will allow those temporaries to
        // be destructed before the call to format_impl.
        return _detail::format_impl(
            writer,
            format_str,
            {_detail::make_format_arg<Writer, _detail::formattable_t<Args>>(args)...});
    }

    /// Write the string format using the given parameters into a fixed-size.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <size_t N, formattable... Args>
    constexpr auto format_to(char (&buffer)[N], string_view format_str, Args const&... args) -> zstring_view {
        fixed_writer writer(buffer);
        format_to(writer, format_str, args...);
        return buffer;
    }

    /// Write the string format using the given parameters and return a string with the result.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a formatted string.
    template <format_appendable ResultT, formattable... Args>
    constexpr auto format_as(string_view format_str, Args const&... args) -> ResultT {
        ResultT result;
        format_append(result, format_str, args...);
        return result;
    }

    /// Format a value into a buffer using the given options.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param value The value to format.
    /// @param options The format control options.
    /// @returns a result code indicating any errors.
    template <format_writable Writer, formattable T>
    constexpr auto format_value_to(Writer& writer, T const& value, string_view spec_string) noexcept(
        noexcept(writer.write({}))) -> format_result {
        return _detail::make_format_arg<Writer>(value).format_into(writer, spec_string);
    }

    /// Format a value into a buffer using the given options.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param value The value to format.
    /// @returns a result code indicating any errors.
    template <format_writable Writer, formattable T>
    constexpr auto format_value_to(Writer& writer, T const& value) noexcept(noexcept(writer.write({})))
        -> format_result {
        return _detail::make_format_arg<Writer>(value).format_into(writer);
    }

    /// Write the string format using the given parameters into a receiver.
    /// @param receiver The receiver of the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <format_appendable Receiver, formattable... Args>
    constexpr auto format_append(Receiver& receiver, string_view format_str, Args const&... args) -> format_result {
        auto writer = append_writer(receiver);
        return format_to(writer, format_str, args...);
    }

    /// Write the string format using the given parameters into a receiver.
    /// @param buffer The text buffer to append to.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <size_t N, formattable... Args>
    constexpr auto format_append(char (&buffer)[N], string_view format_str, Args const&... args) -> zstring_view {
        fixed_writer writer(buffer, stringLength(buffer));
        format_to(writer, format_str, args...);
        return buffer;
    }

    // Write the string format using the given parameters into a receiver.
    /// @param buffer The text buffer to append to.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <formattable... Args>
    constexpr auto format_append(span<char> buffer, string_view format_str, Args const&... args) -> zstring_view {
        fixed_writer writer(buffer.data(), buffer.size());
        format_to(writer, format_str, args...);
        return buffer.data(); // guaranteed NUL-terminated by fixed_writer
    }
} // namespace up
