// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/int_types.h"
#include "potato/spud/string_view.h"

namespace up {
    /// Counted writer
    template <typename OutputT>
    class counted_output final {
    public:
        counted_output(OutputT& output, size_t limit) noexcept : _output(output), _limit(limit) {}

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
} // namespace up
