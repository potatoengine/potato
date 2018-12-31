// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "string_format.h"

gm::format_memory_buffer::~format_memory_buffer() {
    if (_ptr != static_cast<char*>(_buffer)) {
        delete[] _ptr;
    }
}

void gm::format_memory_buffer::push_back(value_type ch) {
    if (_size >= _capacity) {
        auto new_capacity = _capacity * 2;
        while (_size > new_capacity) {
            new_capacity *= 2;
        }

        auto new_buffer = new value_type[new_capacity * 2];

        std::memcpy(new_buffer, _ptr, _size + 1);
        if (_ptr != static_cast<char*>(_buffer)) {
            delete[] _ptr;
        }

        _ptr = new_buffer;
        _capacity = new_capacity;
    }

    _ptr[_size++] = ch;
    _ptr[_size] = '\0';
}
