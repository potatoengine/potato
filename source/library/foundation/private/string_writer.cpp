// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "potato/foundation/string_writer.h"
#include "potato/foundation/string.h"

void up::string_writer::write(value_type ch) {
    _grow(_size + 1);
    _ptr[_size++] = ch;
    _ptr[_size] = '\0';
}

void up::string_writer::write(const_pointer data, size_type length) {
    _grow(_size + length);
    std::memmove(_ptr + _size, data, length);
    _size += length;
    _ptr[_size] = '\0';
}

void up::string_writer::reserve(size_type capacity) {
    if (capacity >= _capacity) {
        size_type newCapacity = capacity + 1;
        UP_ASSERT(newCapacity > capacity, "overflow");

        auto newBuffer = new value_type[newCapacity];

        std::memcpy(newBuffer, _ptr, _size + 1 /*NUL*/);

        if (_ptr != static_cast<char*>(_fixed)) {
            delete[] _ptr;
        }

        _ptr = newBuffer;
        _capacity = newCapacity;
    }
}

auto up::string_writer::acquire(size_type size) -> span<char> {
    _grow(size);
    return {_ptr + _size, _capacity - 1 - _size};
}

void up::string_writer::commit(span<char const> data) {
    UP_ASSERT(data.data() == _ptr + _size, "commit() does not match acquire()d buffer");
    UP_ASSERT(data.size() <= _capacity - 1 - _size, "commit() size exceeds acquired()d buffer");

    _size += data.size();
    _ptr[_size] = '\0';
}

void up::string_writer::reset() {
    if (_ptr != static_cast<char*>(_fixed)) {
        delete[] _ptr;
        _ptr = _fixed;
        _capacity = sizeof(_fixed);
    }
    _size = 0;
    *_ptr = '\0';
}

auto up::string_writer::to_string() const& -> string {
    return string(_ptr, _size);
}

auto up::string_writer::to_string() && -> string {
    if (_ptr != _fixed && _size == _capacity - 1 /*NUL*/) {
        string s = string::take_ownership(_ptr, _size);
        _ptr = nullptr;
        reset();
        return s;
    }
    else {
        string s(_ptr, _size);
        reset();
        return s;
    }
}

void up::string_writer::resize(size_type newSize, value_type fill) {
    if (_size < newSize) {
        _grow(newSize);
        std::memset(_ptr + _size, fill, newSize - _size);
        _size = newSize;
    }
    _size = newSize;
    _ptr[_size] = 0;
}

void up::string_writer::_grow(size_type requiredSize) {
    // >= to account for NUL byte
    if (requiredSize >= _capacity) {
        size_type newCapacity = _capacity * 2;
        while (_size >= newCapacity) {
            newCapacity *= 2;
        }

        auto newBuffer = new value_type[newCapacity];

        std::memcpy(newBuffer, _ptr, _size + 1 /*NUL*/);

        if (_ptr != static_cast<char*>(_fixed)) {
            delete[] _ptr;
        }

        _ptr = newBuffer;
        _capacity = newCapacity;
    }
}
