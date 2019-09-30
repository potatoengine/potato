// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/string.h"
#include <array>

namespace up {

    class UP_RUNTIME_API uuid {
    public:
        constexpr uuid() noexcept : _data{HighLow{}} {}
        constexpr uuid(uuid const& rhs) noexcept : _data{rhs._data.u64} {}
        uuid(up::byte const (&bytes)[16]) noexcept;

        constexpr bool isValid() noexcept { return _data.u64.high != 0 && _data.u64.low != 0; }

        constexpr bool operator==(uuid const& rhs) const noexcept {
            return _data.u64.high == rhs._data.u64.high && _data.u64.low == rhs._data.u64.low;
        }
        constexpr bool operator!=(uuid const& rhs) const noexcept {
            return _data.u64.high != rhs._data.u64.high || _data.u64.low != rhs._data.u64.low;
        }

        static auto generate() noexcept -> uuid;
        static auto toString(const uuid& id) -> string;
        static auto fromString(string_view id) noexcept -> uuid;

        static constexpr uuid zero() noexcept { return uuid{}; }

    private:
        struct HighLow {
            uint64 high = 0;
            uint64 low = 0;
        };

        union Storage {
            HighLow u64;
            byte ub[16];
        };

        Storage _data;
    };

    static_assert(sizeof(uuid) == 16, "sizeof(uuid) must be 16 octects");

} // namespace up
