// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "assertion.h"
#include "span.h"
#include "types.h"
#include "string_view.h"

namespace gm {
    class blob {
    public:
        using pointer = byte*;
        using const_pointer = byte const*;
        using size_type = size_t;

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

        size_type size() const noexcept { return _size; }

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

        span<byte const> as_span() const noexcept { return span{_data, _size}; }
        string_view as_string_view() const noexcept { return string_view{reinterpret_cast<char const*>(_data), _size}; }

    private:
        pointer _data = nullptr;
        size_type _size = 0;
    };
} // namespace gm
