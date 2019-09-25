// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/string.h"
#include <array>

namespace up {

    class UP_RUNTIME_API uuid {
    public:
        using buffer = std::array<up::byte, 16>;

        uuid() noexcept;
        uuid(const uuid& other) noexcept;
        uuid(const buffer& value) noexcept;

        bool isValid() noexcept;

        bool operator==(const uuid& other) const { return _data == other._data; }
        bool operator!=(const uuid& other) const { return !operator==(other); }

        static uuid generate();
        static string toString(const uuid& id);
        static uuid fromString(string_view id);

        static uuid zero();

    private:
        buffer _data;
    };

} // namespace up
