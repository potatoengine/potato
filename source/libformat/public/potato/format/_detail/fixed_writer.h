// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include <cstring>

namespace up {
    /// Writer that appends into a provided memory region, guaranteeing NUL termination and no overflow.
    class fixed_writer final {
    public:
        template <std::size_t Count>
        constexpr fixed_writer(char (&buffer)[Count]) noexcept : _buffer(buffer), _cursor(buffer), _length(Count) {
            *buffer = char{};
        }
        constexpr fixed_writer(char* buffer, std::size_t length) noexcept : _buffer(buffer), _cursor(buffer), _length(length) {
            *buffer = char{};
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
