// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/string.h"
#include <array>

namespace up {

    class UP_RUNTIME_API UUID {
    public:
        constexpr UUID() noexcept : _data{HighLow{}} {}
        constexpr UUID(UUID const& rhs) noexcept : _data{rhs._data.u64} {}
        UUID(up::byte const (&bytes)[16]) noexcept;

        constexpr bool isValid() const noexcept { return _data.u64.high != 0 || _data.u64.low != 0; }

        constexpr bool operator==(UUID const& rhs) const noexcept {
            return _data.u64.high == rhs._data.u64.high && _data.u64.low == rhs._data.u64.low;
        }
        constexpr bool operator!=(UUID const& rhs) const noexcept {
            return _data.u64.high != rhs._data.u64.high || _data.u64.low != rhs._data.u64.low;
        }

        static auto generate() noexcept -> UUID;
        static auto toString(const UUID& id) -> string;
        static auto fromString(string_view id) noexcept -> UUID;

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

    static_assert(sizeof(UUID) == 16, "sizeof(uuid) must be 16 octects");

} // namespace up
