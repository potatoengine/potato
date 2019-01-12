// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "grimm/foundation/assertion.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/vector.h"
#include "grimm/foundation/span.h"

namespace gm {
    class string_writer {
    public:
        using value_type = char;
        using iterator = char*;
        using pointer = char*;
        using const_pointer = char const*;
        using const_iterator = char const*;
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
            GM_ASSERT(*_ptr == '\0', "acquire() operation did not commit()");
            return _ptr;
        }

        /*implicit*/ operator string_view() const noexcept { return {_ptr, _size}; }

        GM_FRAMEWORK_API void write(string_view str);
        GM_FRAMEWORK_API void write(value_type ch);

        // for back_inserter/fmt support
        void push_back(value_type ch) { write(ch); }

        GM_FRAMEWORK_API void reserve(size_type size);

        GM_FRAMEWORK_API span<char> acquire(size_type size);
        GM_FRAMEWORK_API void commit(span<char const> data);

        void clear() {
            *_ptr = 0;
            _size = 0;
        }

        GM_FRAMEWORK_API void reset();

    private:
        void _grow(size_type requiredSize);

        size_t _size = 0;
        size_t _capacity = sizeof(_fixed);
        char* _ptr = _fixed;
        char _fixed[512] = {
            0,
        };
    };
} // namespace gm
