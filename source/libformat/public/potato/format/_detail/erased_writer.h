// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_traits.h"

#include "potato/spud/delegate.h"
#include "potato/spud/string_view.h"

namespace up {
    /// Writer that calls append(data, size) on wrapped value.
    class erased_writer final {
    public:
        template <format_writable WriterT> constexpr erased_writer(WriterT& writer) noexcept : _write([&writer](auto str) { writer.write(str); }) {}

        void write(string_view str) { _write(str); }

    private:
        delegate<void(string_view)> _write;
    };
} // namespace up
