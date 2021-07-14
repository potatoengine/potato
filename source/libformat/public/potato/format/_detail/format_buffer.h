// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "format_write.h"

namespace up::_detail_format {
    class format_iterator;

    class format_buffer {
    public:
        constexpr void push_back(char ch) {
            if (_size == _capacity) {
                grow(_capacity + 1);
            }

            _storage[_size++] = ch;
        }

        constexpr size_t size() const noexcept { return _size; }
        constexpr size_t capacity() const noexcept { return _capacity; }

        constexpr void clear() noexcept { _size = 0; }

    protected:
        constexpr format_buffer(char* storage, size_t capacity) : _storage(storage), _capacity(capacity) {}

        constexpr void set_buffer(char* storage, size_t capacity) {
            _storage = storage;
            _capacity = capacity;
        }

        constexpr virtual void grow(size_t new_capacity) = 0;

    private:
        char* _storage = nullptr;
        size_t _capacity = 0;
        size_t _size = 0;
    };

    template <typename OutputIt>
    class format_iterator_buffer final : public format_buffer {
    public:
        explicit constexpr format_iterator_buffer(OutputIt out) noexcept
            : format_buffer(_buffer, sizeof _buffer)
            , _out(out) {}

        constexpr OutputIt out() noexcept {
            flush();
            return _out;
        }

    private:
        constexpr void grow(size_t) override {
            if (size() == capacity()) {
                flush();
            }
        }

        constexpr void flush() noexcept {
            auto const len = size();
            clear();
            format_write_n(_out, _buffer, len);
        }

        OutputIt _out;
        char _buffer[256] = {
            0,
        };
    };

    template <typename OutputIt>
    struct format_counted_buffer final : public format_buffer {
    public:
        constexpr format_counted_buffer(OutputIt out, size_t limit) noexcept
            : format_buffer(_buffer, sizeof _buffer)
            , _out(out)
            , _limit(limit) {}

        constexpr OutputIt out() noexcept {
            flush();
            return _out;
        }

        constexpr size_t count() const noexcept { return _count + size(); }

    private:
        constexpr void grow(size_t) override {
            if (size() == capacity()) {
                flush();
            }
        }

        constexpr void flush() noexcept {
            auto const len = size();

            size_t const available = _count < _limit ? _limit - _count : 0;
            size_t const advance = len < available ? len : available;

            _count += len;

            clear();

            format_write_n(_out, _buffer, len);
        }

        OutputIt _out;
        char _buffer[256] = {
            0,
        };
        size_t _limit = 0;
        size_t _count = 0; // count of *flushed* characters
    };

    class format_fixed_buffer final : public format_buffer {
    public:
        constexpr format_fixed_buffer(char* out, size_t limit) noexcept
            : format_buffer(out, limit)
            , _out(out)
            , _limit(limit) {}

        constexpr char* out() noexcept {
            flush();
            return _out;
        }

        constexpr size_t count() const noexcept { return _count + size(); }

    private:
        constexpr void grow(size_t) override {
            if (size() == capacity()) {
                flush();
            }
        }

        constexpr void flush() noexcept {
            auto const len = size();

            size_t const available = _count < _limit ? _limit - _count : 0;
            size_t const advance = len < available ? len : available;

            _count += len;

            set_buffer(_overflow, sizeof _overflow);
            clear();

            _out += advance;
        }

        char* _out = nullptr;
        char _overflow[32] = {
            0,
        };
        size_t _limit = 0;
        size_t _count = 0; // count of *flushed* characters
    };

    class format_iterator {
    public:
        constexpr explicit format_iterator(format_buffer& buffer) noexcept : _buffer(&buffer) {}

        constexpr format_iterator& operator*() noexcept { return *this; }
        constexpr format_iterator& operator++() noexcept { return *this; }
        constexpr format_iterator operator++(int) noexcept { return *this; }

        constexpr format_iterator& operator=(char ch) noexcept {
            _buffer->push_back(ch);
            return *this;
        }

    private:
        format_buffer* _buffer = nullptr;
    };

    constexpr format_buffer& make_format_buffer(format_buffer& buffer) noexcept { return buffer; }

    template <is_output_iterator OutputIt>
    constexpr auto make_format_buffer(OutputIt out) noexcept {
        return format_iterator_buffer(out);
    }

    template <is_output_iterator OutputIt>
    constexpr auto make_format_buffer(OutputIt out, size_t limit) noexcept {
        return format_counted_buffer(out, limit);
    }

    template <typename OutputT>
    constexpr auto make_format_buffer(OutputT&& out) noexcept {
        return format_iterator_buffer<OutputT&>(out);
    }

    constexpr auto make_format_buffer(char* out, size_t limit) noexcept { return format_fixed_buffer(out, limit); }
} // namespace up::_detail_format
