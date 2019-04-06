// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/assertion.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/span.h"
#include <cstring>

namespace up {
    class string;

    class string_writer {
    public:
        using value_type = char;
        using iterator = value_type*;
        using pointer = value_type*;
        using const_pointer = value_type const*;
        using const_iterator = value_type const*;
        using size_type = std::size_t;

        string_writer() = default;
        ~string_writer() { reset(); }

        explicit string_writer(size_type capacity) {
            reserve(capacity);
            clear();
        }

        string_writer(string_writer const&) = delete;
        string_writer& operator=(string_writer const&) = delete;

        bool empty() const noexcept { return _size == 0; }
        explicit operator bool() const noexcept { return _size != 0; }

        size_type size() const noexcept { return _size; }
        size_type capacity() const { return _capacity - 1 /*NUL byte*/; }
        const_pointer data() const noexcept { return _ptr; }

        const_pointer c_str() const noexcept {
            UP_ASSERT(_ptr[_size] == '\0', "acquire() operation did not commit()");
            return _ptr;
        }

        /*implicit*/ operator string_view() const noexcept { return {_ptr, _size}; }

        UP_FOUNDATION_API void write(value_type ch);
        UP_FOUNDATION_API void write(const_pointer data, size_type length);
        void write(string_view str) { write(str.data(), str.size()); }

        // for back_inserter/fmt support
        void push_back(value_type ch) { write(ch); }

        UP_FOUNDATION_API void reserve(size_type capacity);

        [[nodiscard]] UP_FOUNDATION_API span<value_type> acquire(size_type size);
        UP_FOUNDATION_API void commit(span<value_type const> data);

        UP_FOUNDATION_API void resize(size_type newSize, value_type fill = ' ');

        void clear() {
            *_ptr = 0;
            _size = 0;
        }

        UP_FOUNDATION_API void reset();

        UP_FOUNDATION_API string to_string() const&;
        UP_FOUNDATION_API string to_string() &&;

    private:
        void _grow(size_type requiredSize);

        size_type _size = 0;
        size_type _capacity = sizeof(_fixed);
        pointer _ptr = _fixed;
        value_type _fixed[512] = {
            0,
        };
    };
} // namespace up
