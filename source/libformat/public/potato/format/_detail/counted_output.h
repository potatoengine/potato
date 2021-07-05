// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_write.h"

#include "potato/spud/int_types.h"
#include "potato/spud/string_view.h"

namespace up {
    /// Counted writer
    template <typename OutputT>
    class counted_output final {
    public:
        counted_output(OutputT& output, size_t limit) noexcept : _output(output), _limit(limit) {}

        constexpr void append(char const* str, size_t size) noexcept(is_format_write_noexcept<OutputT>) {
            _count += size;

            if (size <= _limit) {
                format_write_n(_output, str, size);
                _limit -= size;
            }
            else {
                format_write_n(_output, str, _limit);
                _limit = 0;
            }
        }

        constexpr OutputT& current() noexcept { return _output; }
        constexpr size_t count() noexcept { return _count; }

    private:
        OutputT& _output;
        size_t _limit = 0;
        size_t _count = 0;
    };
} // namespace up
