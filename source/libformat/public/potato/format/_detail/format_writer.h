// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include <potato/spud/string_view.h>

/// Interface for any buffer that the format library can write into.
class up::format_writer {
public:
    virtual ~format_writer() = default;

    /// Write a string slice.
    /// @param str The string to write.
    virtual void write(string_view str) = 0;
};
