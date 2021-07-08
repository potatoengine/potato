// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/concepts.h"
#include "potato/spud/string_view.h"
#include "potato/spud/traits.h"

namespace up {
    /// Generic formatter
    template <typename T>
    struct formatter {
        // constexpr char const* parse(format_parse_context& ctx) noexcept;

        // template <typename ContextT>
        // constexpr void format(T const& value, ContextT& ctx);
    };

    /// Formatter parse context
    class format_parse_context {
    public:
        constexpr explicit format_parse_context(char const* begin, char const* end) noexcept
            : _begin(begin)
            , _end(end) {}

        constexpr char const* begin() const noexcept { return _begin; }
        constexpr char const* end() const noexcept { return _end; }

        constexpr void advance_to(char const* ptr) noexcept { _begin = ptr; }

    private:
        char const* _begin = nullptr;
        char const* _end = nullptr;
    };

    /// Formatter format context
    template <typename OutputT>
    class format_context {
    public:
        constexpr explicit format_context(OutputT& output) noexcept : _output(output) {}

        constexpr OutputT& out() noexcept { return _output; }

    private:
        OutputT& _output;
    };

    /// Formatter for void types
    template <>
    struct formatter<void> {
        constexpr char const* parse(format_parse_context& ctx) noexcept { return ctx.begin(); }

        template <typename ContextT, typename ValueT>
        constexpr void format(ValueT const&, ContextT&) {}
    };
} // namespace up
