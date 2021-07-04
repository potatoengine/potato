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

        constexpr void append(string_view text) noexcept(is_format_write_noexcept<OutputT>) {
            auto const size = text.size();

            if (size <= _limit) {
                format_write(_output, text);
                _limit -= size;
            }
            else {
                format_write(_output, text.first(_limit));
                _limit = 0;
            }
        }

        constexpr OutputT& current() noexcept { return _output; }

    private:
        OutputT& _output;
        size_t _limit = 0;
    };
} // namespace up
