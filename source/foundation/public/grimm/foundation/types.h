// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#pragma once

#include <cinttypes>
#include <cstddef>

namespace gm {
    inline namespace types {
        using int8 = std::int8_t;
        using uint8 = std::uint8_t;

        using int16 = std::int16_t;
        using uint16 = std::uint16_t;

        using int32 = std::int32_t;
        using uint32 = std::uint32_t;

        using int64 = std::int64_t;
        using uint64 = std::uint64_t;

        using uint = unsigned int;

        using intptr = std::intptr_t;
        using uintptr = std::uintptr_t;

        using byte = std::byte;
    } // namespace types
} // namespace gm

using namespace gm::types;
