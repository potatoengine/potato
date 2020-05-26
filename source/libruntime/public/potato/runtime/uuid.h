// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/string.h"

#include <array>

namespace up {

    class UP_RUNTIME_API UUID {
    public:
        static constexpr int octects = 16;
        using Bytes = up::byte[octects];

        constexpr UUID() noexcept : _data{HighLow{}} {}
        constexpr UUID(UUID const& rhs) noexcept : _data{rhs._data.u64} {}
        constexpr UUID(Bytes const& bytes) noexcept;

        constexpr bool isValid() const noexcept { return _data.u64.high != 0 || _data.u64.low != 0; }

        constexpr bool operator==(UUID const& rhs) const noexcept {
            return _data.u64.high == rhs._data.u64.high && _data.u64.low == rhs._data.u64.low;
        }
        constexpr bool operator!=(UUID const& rhs) const noexcept {
            return _data.u64.high != rhs._data.u64.high || _data.u64.low != rhs._data.u64.low;
        }

        auto toString() const -> string;

        static auto generate() noexcept -> UUID;
        static auto fromString(string_view id) noexcept -> UUID;

    private:
        struct HighLow {
            uint64 high = 0;
            uint64 low = 0;
        };

        union Storage {
            HighLow u64;
            byte ub[octects];
        };

        Storage _data;
    };

    static_assert(sizeof(UUID) == UUID::octects, "sizeof(uuid) must be 16 octects");

} // namespace up
