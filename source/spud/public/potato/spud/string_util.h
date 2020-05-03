// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "platform.h"
#include "int_types.h"

namespace up {
    inline constexpr size_t stringLength(char const* str) noexcept {
        return __builtin_strlen(str);
    }

    inline constexpr int stringCompare(char const* left, char const* right, size_t length) noexcept {
        return __builtin_memcmp(left, right, length);
    }

    inline constexpr char const* stringFindChar(char const* str, size_t length, char ch) noexcept {
#if defined(UP_COMPILER_GCC)
        return (char const*)__builtin_memchr(str, ch, length);
#else
        return __builtin_char_memchr(str, ch, length);
#endif
    }

    inline auto as_char(char8_t const* u8str) -> char const* { return reinterpret_cast<char const*>(u8str); }
} // namespace up
