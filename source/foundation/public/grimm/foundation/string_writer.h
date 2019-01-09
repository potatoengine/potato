// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/string_view.h"
#include "grimm/foundation/vector.h"

namespace gm {
    class string_writer {
    public:
        using value_type = char;
        using iterator = char*;
        using pointer = char*;
        using const_pointer = char const*;
        using const_iterator = char const*;
        using size_type = std::size_t;

        string_writer() { clear(); }
        explicit string_writer(size_type capacity) {
            reserve(capacity);
            clear();
        }

        bool empty() const noexcept { return _buffer.size() <= 1; }
        explicit operator bool() const noexcept { return _buffer.size() > 1; }

        size_type size() const noexcept { return _buffer.size() - 1; }
        const_pointer data() const noexcept { return _buffer.data(); }

        const_pointer c_str() const noexcept { return _buffer.data(); }

        /*implicit*/ operator string_view() const noexcept { return {_buffer.data(), _buffer.size() - 1}; }

        void write(string_view str) {
            _buffer.pop_back();
            _buffer.insert(_buffer.end(), str.begin(), str.end());
            _buffer.push_back('\0');
        }

        void write(value_type ch) {
            _buffer.pop_back();
            _buffer.push_back(ch);
            _buffer.push_back('\0');
        }

        void reserve(size_type size) {
            // FIXME: overflow?
            _buffer.reserve(size + 1);
        }

        void clear() {
            _buffer.clear();
            _buffer.push_back('\0');
        }

    private:
        vector<char> _buffer;
    };
} // namespace gm
