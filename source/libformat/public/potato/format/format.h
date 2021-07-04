// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_detail/counted_output.h"
#include "_detail/format_arg.h"
#include "_detail/format_impl.h"
#include "_detail/format_traits.h"
#include "_detail/format_write.h"
#include "_detail/formatter.h"
#include "_detail/formatter_float.h"
#include "_detail/formatter_int.h"
#include "_detail/formatter_string.h"

#include "potato/spud/string_view.h"

#include <type_traits>

namespace up {
    /// Write the string format using the given arguments into a buffer.
    /// @param output The output iterator or writeable buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args Packaged arguments to use for formatting.
    /// @returns a result code indicating any errors.
    template <typename OutputT>
    constexpr decltype(auto) vformat_to(OutputT&& output, string_view format_str, format_args&& args) {
        return _detail::format_impl(
            _detail::format_impl_context<OutputT>{output, format_str.begin(), format_str.end(), args});
    }

    /// Write the string format using the given arguments into a buffer.
    /// @param output The output iterator or writeable buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <typename OutputT, formattable... Args>
    constexpr decltype(auto) format_to(OutputT&& output, string_view format_str, Args const&... args) {
        return vformat_to(output, format_str, {make_format_arg<std::remove_reference_t<OutputT>, Args>(args)...});
    }

    /// Result type of format_to_n
    template <typename OutputT>
    struct format_to_n_result {
        OutputT out;
        size_t size = 0;
    };

    /// Write the string format using the given arguments into a buffer up to a given size.
    /// @param output The write buffer that will receive the formatted text.
    /// @param count The maximum number of characters to write.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a format_to_n_result with the output iterator and count that would have been written.
    template <typename OutputT, formattable... Args>
    constexpr format_to_n_result<OutputT> format_to_n(
        OutputT&& output,
        size_t count,
        string_view format_str,
        Args const&... args) {
        using CountedT = counted_output<std::remove_reference_t<OutputT>>;
        CountedT counted(output, count);
        format_to(counted, format_str, args...);
        return {counted.current(), counted.count()};
    }

    /// Write the string format using the given arguments into a fixed-size.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <size_t N, formattable... Args>
    constexpr char* format_to(char (&buffer)[N], string_view format_str, Args const&... args) {
        auto const [end, _] = format_to_n(static_cast<char*>(buffer), N - 1 /*NUL*/, format_str, args...);
        *end = '\0';
        return end;
    }

    /// Write the string format using the given parameters and return a string with the result.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a formatted string.
    template <typename ResultT, formattable... Args>
    constexpr ResultT format_as(string_view format_str, Args const&... args) {
        ResultT result;
        format_to(result, format_str, args...);
        return result;
    }
} // namespace up
