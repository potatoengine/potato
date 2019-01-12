// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "string_writer.h"

void gm::string_writer::write(string_view str) {
    _grow(_size + str.size());
    std::memmove(_ptr + _size, str.data(), str.size());
    _size += str.size();
    _ptr[_size] = '\0';
}

void gm::string_writer::write(value_type ch) {
    _grow(_size + 1);
    _ptr[_size++] = ch;
    _ptr[_size] = '\0';
}

void gm::string_writer::reserve(size_type size) {
    _grow(size);
}

auto gm::string_writer::acquire(size_type size) -> span<char> {
    _grow(size);
    return {_ptr + _size, _capacity - 1 - _size};
}

void gm::string_writer::commit(span<char const> data) {
    GM_ASSERT(data.data() == _ptr + _size, "commit() does not match acquire()d buffer");
    GM_ASSERT(data.size() <= _capacity - 1  - _size, "commit() size exceeds acquired()d buffer");

    _size += data.size();
    _ptr[_size] = '\0';
}

void gm::string_writer::reset() {
    if (_ptr != static_cast<char*>(_fixed)) {
        delete[] _ptr;
        _ptr = _fixed;
        _capacity = sizeof(_fixed);
    }
    _size = 0;
    *_ptr = '\0';
}

void gm::string_writer::_grow(size_type requiredSize) {
    // >= to account for NUL byte
    if (requiredSize >= _capacity) {
        size_type newCapacity = _capacity * 2;
        while (_size >= newCapacity) {
            newCapacity *= 2;
        }

        auto newBuffer = new value_type[newCapacity];

        std::memcpy(newBuffer, _ptr, _size + 1/*NUL*/);
        if (_ptr != static_cast<char*>(_fixed)) {
            delete[] _ptr;
        }

        _ptr = newBuffer;
        _capacity = newCapacity;
    }
}
