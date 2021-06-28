// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_detail/format_arg.h"
#include "_detail/format_arg_impl.h"
#include "_detail/format_impl.h"
#include "_detail/format_result.h"
#include "_detail/format_traits.h"
#include "_detail/format_write.h"

#include "potato/spud/string_view.h"

#include <type_traits>

namespace up {
    /// Counted writer
    template <typename OutputT>
    class writer_counted final {
    public:
        writer_counted(OutputT& output, size_t limit) noexcept : _output(output), _limit(limit) {}

        constexpr void write(string_view text) {
            if (text.size() <= _limit) {
                format_write(_output, text);
                _limit -= text.size();
            }
            else {
                format_write(_output, text.first(_limit));
                _limit = 0;
            }
        }

    private:
        OutputT& _output;
        size_t _limit = 0;
    };

    /// Default format helpers.
    template <typename Writer>
    constexpr void format_value(Writer& out, string_view str) noexcept(noexcept(format_write(out, str))) {
        format_write(out, str);
    }

    /// Write the string format using the given arguments into a buffer.
    /// @param output The output iterator or writeable buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args Packaged arguments to use for formatting.
    /// @returns a result code indicating any errors.
    template <typename OutputT>
    constexpr auto vformat_to(OutputT&& output, string_view format_str, format_args&& args) {
        return _detail::format_impl(
            _detail::format_context<OutputT>{output, format_str.begin(), format_str.end(), args});
    }

    /// Write the string format using the given arguments into a buffer.
    /// @param output The output iterator or writeable buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <typename OutputT, formattable... Args>
    constexpr auto format_to(OutputT&& output, string_view format_str, Args const&... args) -> format_result {
        return vformat_to(
            output,
            format_str,
            {make_format_arg<std::remove_reference_t<OutputT>, _detail::formattable_t<Args>>(args)...});
    }

    /// Write the string format using the given arguments into a buffer up to a given size.
    /// @param output The write buffer that will receive the formatted text.
    /// @param count The maximum number of characters to write.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <typename OutputT, formattable... Args>
    constexpr auto format_to_n(OutputT&& output, size_t count, string_view format_str, Args const&... args)
        -> format_result {
        using CountedT = writer_counted<std::remove_reference_t<OutputT>>;
        CountedT counted(output, count);
        return vformat_to(counted, format_str, {make_format_arg<CountedT, _detail::formattable_t<Args>>(args)...});
    }

    /// Write the string format using the given arguments into a fixed-size.
    /// @param writer The write buffer that will receive the formatted text.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a result code indicating any errors.
    template <size_t N, formattable... Args>
    constexpr auto format_to(char (&buffer)[N], string_view format_str, Args const&... args) {
        return format_to_n(static_cast<char*>(buffer), N - 1 /*NUL*/, format_str, args...);
    }

    /// Write the string format using the given parameters and return a string with the result.
    /// @param format_str The primary text and formatting controls to be written.
    /// @param args The arguments used by the formatting string.
    /// @returns a formatted string.
    template <typename ResultT, formattable... Args>
    constexpr auto format_as(string_view format_str, Args const&... args) -> ResultT {
        ResultT result;
        format_to(result, format_str, args...);
        return result;
    }
} // namespace up
