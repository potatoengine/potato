// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string.h"
#include <array>

namespace up {

    class UP_FOUNDATION_API uuid {
    public:
        using buffer = std::array<up::byte, 16>;

        uuid() noexcept;
        uuid(const uuid& other) noexcept;
        uuid(const buffer& value) noexcept;

        bool isValid() noexcept;

        bool operator==(const uuid& other) const { return std::memcmp(&_data, &other._data, sizeof(_data)) == 0; }
        bool operator!=(const uuid& other) const { return !operator==(other); }

        static uuid generate();
        static string toString(const uuid& id);

        static uuid zero();

        buffer _data;
    };

} // namespace up
