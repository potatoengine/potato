// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "int_types.h"

namespace gm {
    inline constexpr size_t stringLength(char const* str) noexcept {
        return __builtin_strlen(str);
    }

    inline constexpr int stringCompare(char const* left, char const* right, size_t length) noexcept {
        return __builtin_memcmp(left, right, length);
    }

    inline constexpr char const* stringFindChar(char const* str, size_t length, char ch) noexcept {
        return __builtin_char_memchr(str, ch, length);
    }
} // namespace gm
