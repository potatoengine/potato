// Copyright (C) 2014,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <iostream>

namespace gm {
    class string;
    class string_view;
    class zstring_view;
    template <size_t S>
    class fixed_string;

    template <typename C>
    inline std::ostream& operator<<(std::basic_ostream<C>& os, string_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::ostream& operator<<(std::basic_ostream<C>& os, zstring_view sv) {
        return os.write(sv.data(), sv.size());
    }

    template <typename C>
    inline std::ostream& operator<<(std::basic_ostream<C>& os, string s) {
        return os.write(s.data(), s.size());
    }

    template <size_t S>
    inline std::ostream& operator<<(std::ostream& os, fixed_string<S> const& fs) {
        os.write(fs.data(), fs.size());
        return os;
    }
} // namespace gm
