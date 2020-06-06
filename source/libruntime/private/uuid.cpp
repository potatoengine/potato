// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "uuid.h"
#include "assertion.h"

#include "potato/spud/ascii.h"
#include "potato/spud/string_writer.h"

constexpr up::UUID::UUID(Bytes const& bytes) noexcept : _data{HighLow{}} {
    for (unsigned i = 0; i != sizeof(bytes); ++i) {
        _data.ub[i] = bytes[i];
    }
}

constexpr char byteToString(up::byte byte) noexcept { return static_cast<char>(byte); }

auto up::UUID::toString() const -> string {
    // format 9554084e-4100-4098-b470-2125f5eed133
    string_writer buffer;
    format_append(buffer,
        "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
        _data.ub[0], // NOLINT(readability-magic-numbers)
        _data.ub[1], // NOLINT(readability-magic-numbers)
        _data.ub[2], // NOLINT(readability-magic-numbers)
        _data.ub[3], // NOLINT(readability-magic-numbers)
        _data.ub[4], // NOLINT(readability-magic-numbers)
        _data.ub[5], // NOLINT(readability-magic-numbers)
        _data.ub[6], // NOLINT(readability-magic-numbers)
        _data.ub[7], // NOLINT(readability-magic-numbers)
        _data.ub[8], // NOLINT(readability-magic-numbers)
        _data.ub[9], // NOLINT(readability-magic-numbers)
        _data.ub[10], // NOLINT(readability-magic-numbers)
        _data.ub[11], // NOLINT(readability-magic-numbers)
        _data.ub[12], // NOLINT(readability-magic-numbers)
        _data.ub[13], // NOLINT(readability-magic-numbers)
        _data.ub[14], // NOLINT(readability-magic-numbers)
        _data.ub[15]); // NOLINT(readability-magic-numbers)

    return buffer.c_str();
}

auto up::UUID::fromString(string_view id) noexcept -> UUID {
    if (!id.empty() && id.front() == '{' && id.back() == '}') {
        id = id.substr(1, id.size() - 2);
    }

    byte next = {};
    bool octect = false;

    UUID result;
    int bidx = 0;

    for (auto c : id) {
        if (c == '-') {
            continue;
        }

        int digit = ascii::from_hex(c);
        if (digit == -1) {
            return UUID{};
        }

        next <<= 4;
        next |= static_cast<byte>(digit);

        if (octect) {
            if (bidx == sizeof(result._data.ub)) {
                return UUID{};
            }
            result._data.ub[bidx++] = next;
        }

        octect = !octect;
    }

    if (bidx != sizeof(result._data.ub) || octect) { // NOLINT(readability-magic-numbers)
        return UUID{};
    }

    return result;
}
