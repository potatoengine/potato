// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

namespace up {
    /// Formatter format context
    template <typename OutputT>
    class format_context {
    public:
        constexpr explicit format_context(OutputT& output) noexcept : _output(output) {}

        constexpr OutputT& out() noexcept { return _output; }

    private:
        OutputT& _output;
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
} // namespace up
