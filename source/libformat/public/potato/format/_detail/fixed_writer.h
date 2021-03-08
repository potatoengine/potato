// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/string_view.h"

#include <cstring>

namespace up {
    /// Writer that appends into a provided memory region, guaranteeing NUL termination and no overflow.
    class fixed_writer final {
    public:
        template <std::size_t Count>
        constexpr fixed_writer(char (&buffer)[Count], size_t offset = 0) noexcept
            : _buffer(buffer)
            , _cursor(buffer + offset)
            , _length(Count) {
            *_cursor = '\0';
        }

        constexpr fixed_writer(char* buffer, size_t length) noexcept
            : _buffer(buffer)
            , _cursor(buffer)
            , _length(length) {
            *_cursor = '\0';
        }

        void write(string_view str) noexcept {
            std::size_t const size = _cursor - _buffer;
            std::size_t const capacity = _length - 1 /*NUL*/;
            std::size_t const available = capacity - size;
            std::size_t const length = available < str.size() ? available : str.size();

            std::memcpy(_cursor, str.data(), length);
            _cursor += length;
            *_cursor = char{};
        }

    private:
        char* _buffer = nullptr;
        char* _cursor = nullptr;
        std::size_t _length = 0;
    };
} // namespace up
