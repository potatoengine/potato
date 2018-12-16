// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.

#pragma once

#include "debug.h"
#include "string_view.h"
#include <iterator>

// needed by fmtlib
#if !defined(FMT_ASSERT)
#    define FMT_ASSERT(cond, message)                                    \
        do {                                                             \
            if (!(cond)) {                                               \
                ::gm::fatal_error(__FILE__, __LINE__, #cond, (message)); \
                std::abort();                                            \
            }                                                            \
        } while (false)
#endif
#if !defined(assert)
#    define assert(cond) \
        FMT_ASSERT((cond), "assertion failed")
#endif

#include <fmt/format.h>

namespace gm {
    class format_memory_buffer {
    public:
        using value_type = char;
        using size_type = size_t;

        format_memory_buffer() = default;
        ~format_memory_buffer();

        format_memory_buffer(format_memory_buffer const&) = delete;
        format_memory_buffer& operator=(format_memory_buffer const&) = delete;

        value_type const* c_str() const { return _ptr; }
        value_type const* data() const { return _ptr; }
        size_type size() const { return _size; }
        size_type capacity() const { return _capacity; }
        operator string_view() const { return string_view(_ptr, _size); }

        void push_back(value_type ch);

    private:
        size_t _size = 0;
        size_t _capacity = sizeof(_buffer) - 1;
        char* _ptr = _buffer;
        char _buffer[512] = {
            '\0',
        };
    };

    template <size_t Capacity>
    class format_fixed_buffer {
    public:
        using value_type = char;
        using size_type = size_t;

        format_fixed_buffer() = default;
        ~format_fixed_buffer() = default;

        format_fixed_buffer(format_fixed_buffer const&) = delete;
        format_fixed_buffer& operator=(format_fixed_buffer const&) = delete;

        value_type const* c_str() const { return _buffer; }
        value_type const* data() const { return _buffer; }
        size_type size() const { return _size; }
        size_type capacity() const { return Capacity - 1; }
        operator string_view() const { return string_view(_buffer, _size); }

        void push_back(value_type ch);

    private:
        size_t _size = 0;
        char _buffer[Capacity] = {
            '\0',
        };
    };

    template <size_t Capacity>
    void format_fixed_buffer<Capacity>::push_back(value_type ch) {
        if (_size < Capacity - 1) {
            _buffer[_size++] = ch;
            _buffer[_size] = '\0';
        }
    }

    template <typename... Args>
    constexpr decltype(auto) format(string_view format, Args const&... args) {
        return ::fmt::vformat(::fmt::string_view(format.data(), format.size()), ::fmt::make_format_args(args...));
    }

    template <typename Buffer, typename... Args>
    constexpr decltype(auto) format_into(Buffer& buffer, string_view format, Args const&... args) {
        return ::fmt::format_to(std::back_inserter(buffer), ::fmt::string_view(format.data(), format.size()), args...);
    }
} // namespace gm
