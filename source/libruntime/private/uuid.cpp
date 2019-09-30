// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/spud/string_writer.h"
#include "potato/spud/ascii.h"

up::UUID::UUID(up::byte const (&bytes)[16]) noexcept : _data{HighLow{}} {
    for (int i = 0; i != 16; ++i) {
        _data.ub[i] = bytes[i];
    }
}

constexpr char byteToString(up::byte byte) noexcept {
    return static_cast<char>(byte);
}

auto up::UUID::toString(const UUID& id) -> string {
    // format 9554084e-4100-4098-b470-2125f5eed133
    string_writer buffer;
    format_into(buffer, "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                id._data.ub[0], id._data.ub[1], id._data.ub[2], id._data.ub[3],
                id._data.ub[4], id._data.ub[5],
                id._data.ub[6], id._data.ub[7],
                id._data.ub[8], id._data.ub[9],
                id._data.ub[10], id._data.ub[11], id._data.ub[12], id._data.ub[13], id._data.ub[14], id._data.ub[15]);

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
            return UUID::zero();
        }

        next <<= 4;
        next |= byte{digit};

        if (octect) {
            if (bidx == 16) {
                return zero();
            }
            result._data.ub[bidx++] = next;
        }

        octect = !octect;
    }

    if (bidx != 16 || octect) {
        return zero();
    }

    return result;
}
