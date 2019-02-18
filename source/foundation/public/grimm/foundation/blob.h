// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "assertion.h"
#include "span.h"
#include "types.h"

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

        blob() = default;
        explicit blob(size_type size) : _size(size) {
            _data = new byte[size];
        }
        explicit blob(view<byte> data) : blob(data.size()) {
            std::memcpy(_data, data.data(), _size);
        }

        blob(blob&& rhs) : _data(rhs._data), _size(rhs._size) {
            rhs._data = nullptr;
            rhs._size = 0;
        }

        ~blob() {
            delete[] _data;
        }

        blob& operator=(blob&& rhs) noexcept {
            _size = rhs._size;
            _data = rhs.release();
            return *this;
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
            delete[] _data;
            _data = nullptr;
            _size = 0;
        }

        [[nodiscard]] pointer release() noexcept {
            pointer tmp = _data;
            _data = nullptr;
            _size = 0;
            return tmp;
        }

        /*implicit*/ operator span<byte>() noexcept { return {reinterpret_cast<pointer_bytes>(_data), _size}; }
        /*implicit*/ operator span<byte const>() const noexcept { return {reinterpret_cast<const_pointer_bytes>(_data), _size}; }

    private:
        pointer _data = nullptr;
        size_type _size = 0;
    };
} // namespace gm
