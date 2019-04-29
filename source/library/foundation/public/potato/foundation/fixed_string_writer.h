// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <cstring>

namespace up {
    template <std::size_t Capacity = 512>
    class fixed_string_writer {
    public:
        using value_type = char;
        using iterator = value_type*;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using const_iterator = value_type const*;
        using size_type = std::size_t;

        fixed_string_writer() = default;
        ~fixed_string_writer() = default;

        fixed_string_writer(fixed_string_writer const&) = delete;
        fixed_string_writer& operator=(fixed_string_writer const&) = delete;

        bool empty() const noexcept { return _size == 0; }
        explicit operator bool() const noexcept { return _size != 0; }

        size_type size() const noexcept { return _size; }
        size_type capacity() const { return Capacity - 1 /*NUL byte*/; }
        const_pointer data() const noexcept { return _buffer; }

        const_pointer c_str() const noexcept { return _buffer; }

        /*implicit*/ operator string_view() const noexcept { return {_buffer, _size}; }

        void write(string_view str) {
            size_type available = capacity() - _size;
            size_type writeLen = available < str.size() ? available : str.size();
            std::memmove(_buffer + _size, str.data(), writeLen);
            _buffer[_size += writeLen] = 0;
        }

        void write(value_type ch) {
            if (_size < capacity()) {
                _buffer[_size] = ch;
                _buffer[++_size] = 0;
            }
        }

        // for back_inserter/fmt support
        void push_back(value_type ch) { write(ch); }

        void clear() {
            *_buffer = 0;
            _size = 0;
        }

    private:
        size_type _size = 0;
        value_type _buffer[Capacity] = {
            0,
        };
    };
} // namespace up
