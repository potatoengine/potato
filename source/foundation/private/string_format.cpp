// Copyright (C) 2014,2015 Sean Middleditch, all rights reserverd.
//
// Logging system is designed to minimize the overhead esp. in debug scenarios (Where we most likely want logging), could probably be better.

#include "string_format.h"

void gm::format_fixed_buffer::push_back(value_type ch)
{
    if (_size < Capacity - 1)
    {
        _buffer[_size++] = ch;
        _buffer[_size] = '\0';
    }
}