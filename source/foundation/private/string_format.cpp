// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "string_format.h"

gm::format_memory_buffer::~format_memory_buffer()
{
    if (_ptr != _buffer)
        delete[] _ptr;
}

void gm::format_memory_buffer::push_back(value_type ch)
{
    if (_size >= _capacity)
    {
        value_type* new_buffer = new value_type[_capacity * 2];
        std::memcpy(new_buffer, _ptr, _size + 1);
        if (_ptr != _buffer)
            delete[] _ptr;
        _ptr = new_buffer;
    }

    _buffer[_size++] = ch;
    _buffer[_size] = '\0';
}