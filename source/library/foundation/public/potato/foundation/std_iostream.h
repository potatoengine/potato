// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <iostream>

namespace up {
    class string;
    class string_view;
    class zstring_view;
    template <size_t S>
    class fixed_string;

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, string_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, zstring_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, string const& s) {
        return os.write(s.data(), s.size());
    }

    template <typename C, size_t S>
    inline std::basic_ostream<C>& operator<<(std::basic_ostream<C>& os, fixed_string<S> const& fs) {
        os.write(fs.data(), fs.size());
        return os;
    }
} // namespace up
