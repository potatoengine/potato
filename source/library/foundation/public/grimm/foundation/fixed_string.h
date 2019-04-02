// Copyright (C) 2018,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "string_view.h"
#include <cstring>

namespace gm {
    template <size_t Capacity>
    class fixed_string {
        static_assert(Capacity > 0);

    public:
        using value_type = char;
        using pointer = char const*;
        using size_type = size_t;
        static constexpr auto effective_capacity = Capacity - 1;

        fixed_string() = default;
        ~fixed_string() = default;

        inline fixed_string(fixed_string const& string);
        inline fixed_string(string_view string);

        inline fixed_string& operator=(fixed_string const& string);
        inline fixed_string& operator=(string_view string);

        /*implicit*/ operator string_view() const { return {_buffer, _size}; }

        explicit operator bool() const { return _size != 0; }
        bool empty() const { return _size == 0; }

        size_type size() const { return _size; }
        size_type capacity() const { return effective_capacity; }

        pointer data() const { return _buffer; }
        pointer c_str() const { return _buffer; }

        inline void clear();

        template <typename Writer, typename Spec>
        friend void format_value(Writer& writer, fixed_string const& fs, Spec const& options) noexcept {
            format_value_to(writer, string_view{fs._buffer, fs._size}, options);
        }

    private:
        size_t _size = 0;
        char _buffer[Capacity] = {
            '\0',
        };
    };

    template <size_t Capacity>
    fixed_string<Capacity>::fixed_string(fixed_string const& string) : _size(string._size) {
        std::memcpy(_buffer, string._buffer, _size + 1);
    }

    template <size_t Capacity>
    fixed_string<Capacity>::fixed_string(string_view string) {
        *this = string;
    }

    template <size_t Capacity>
    auto fixed_string<Capacity>::operator=(fixed_string const& string) -> fixed_string& {
        if (this != &string) {
            _size = string._size;
            std::memcpy(_buffer, string._buffer, _size + 1);
        }
        return *this;
    }

    template <size_t Capacity>
    auto fixed_string<Capacity>::operator=(string_view string) -> fixed_string& {
        size_t newSize = effective_capacity < string.size() ? effective_capacity : string.size();
        _size = newSize;

        if (string.data() >= _buffer && string.data() < _buffer + Capacity) {
            std::memmove(_buffer, string.data(), newSize);
        }
        else {
            std::memcpy(_buffer, string.data(), newSize);
        }
        _buffer[newSize] = '\0';
        return *this;
    }

    template <size_t Capacity>
    void fixed_string<Capacity>::clear() {
        _size = 0;
        _buffer[0] = '\0';
    }

    template <typename HashAlgorithm, size_t Size>
    void hash_append(HashAlgorithm& hasher, fixed_string<Size> const& string) {
        hasher(string.data(), string.size());
    }
} // namespace gm
