// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/hash.h"
#include "potato/spud/string.h"
#include "potato/spud/string_format.h"

#include <array>

namespace up {

    class UP_RUNTIME_API UUID {
    public:
        static constexpr int octects = 16;
        static constexpr int strLength = 37; // 32 hex values, four dashes, and NUL
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

        template <format_writable FormatterT>
        friend void format_value(FormatterT& writer, UUID uuid) {
            // format 9554084e-4100-4098-b470-2125f5eed133
            format_to(
                writer,
                "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                uuid._data.ub[0],
                uuid._data.ub[1],
                uuid._data.ub[2],
                uuid._data.ub[3],
                uuid._data.ub[4],
                uuid._data.ub[5],
                uuid._data.ub[6],
                uuid._data.ub[7],
                uuid._data.ub[8],
                uuid._data.ub[9],
                uuid._data.ub[10],
                uuid._data.ub[11],
                uuid._data.ub[12],
                uuid._data.ub[13],
                uuid._data.ub[14],
                uuid._data.ub[15]);
        }

        template <typename HashAlgorithm = default_hash>
        friend uint64 hash_value(UUID uuid) noexcept {
            HashAlgorithm hasher{};
            hash_append(hasher, uuid._data.u64.high);
            hash_append(hasher, uuid._data.u64.low);
            return hasher.finalize();
        }

        static auto generate() noexcept -> UUID;
        static auto fromString(string_view id) noexcept -> UUID;

    private:
        struct HighLow {
            uint64 high = 0;
            uint64 low = 0;
        };

        union Storage {
            HighLow u64 = {};
            byte ub[octects];
        };

        Storage _data = {HighLow{}};
    };

    static_assert(sizeof(UUID) == UUID::octects, "sizeof(uuid) must be 16 octects");

} // namespace up
