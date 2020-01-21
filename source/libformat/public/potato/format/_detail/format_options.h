// Copyright (C) 2020 Sean Middleditch, all rights reserverd.

#pragma once

#include <type_traits>

/// Extra formatting specifications.
namespace up {
    class format_options {
    public:
        constexpr format_options() noexcept : alternate_form(false), leading_zeroes(false) {}

        unsigned char width = 0;
        unsigned char precision = ~static_cast<unsigned char>(0);
        char specifier = 0;
        bool alternate_form : 1;
        bool leading_zeroes : 1;
    };

    static_assert(std::is_trivially_copyable_v<format_options>);
    static_assert(sizeof(format_options) == 4);
} // namespace up
