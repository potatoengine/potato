// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "allocator.h"

namespace gm {
    class blob {
    public:
        using pointer = void*;
        using const_pointer = void const*;
        using pointer_bytes = std::byte*;
        using const_pointer_bytes = std::byte const*;
        using pointer_chars = char*;
        using const_pointer_chars = char const*;
        using size_type = std::size_t;

        static constexpr size_type alignment = alignof(double);

        blob() = default;
        explicit blob(size_type size) : _size(size) {
            _data = default_allocator{}.allocate(size, alignment);
        }

        blob(blob&& rhs) : _data(rhs._data), _size(rhs._size) {
            rhs._data = nullptr;
            rhs._size = 0;
        }

        ~blob() {
            default_allocator{}.deallocate(_data, _size, 32);
        }

        pointer data() noexcept { return _data; }
        const_pointer data() const noexcept { return _data; }

        pointer_bytes data_bytes() noexcept { return reinterpret_cast<pointer_bytes>(_data); }
        const_pointer_bytes data_bytes() const noexcept { return reinterpret_cast<const_pointer_bytes>(_data); }

        pointer_chars data_chars() noexcept { return reinterpret_cast<pointer_chars>(_data); }
        const_pointer_chars data_chars() const noexcept { return reinterpret_cast<const_pointer_chars>(_data); }

        size_type size() const noexcept { return _size; }
        size_type size_bytes() const noexcept { return _size; }

        bool empty() const noexcept { return _size == 0; }
        explicit operator bool() const noexcept { return _size != 0; }

        void reset() {
            default_allocator{}.deallocate(_data, _size, alignment);
            _data = nullptr;
            _size = 0;
        }

        [[nodiscard]] pointer release() noexcept {
            pointer tmp = _data;
            _data = nullptr;
            _size = 0;
            return tmp;
        }

    private:
        pointer _data = nullptr;
        size_type _size = 0;
    };
} // namespace gm
