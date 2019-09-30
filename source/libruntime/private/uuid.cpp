// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/spud/string_writer.h"

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

static auto isValidChar(const char c) -> bool {
    if (c >= '0' && c <= '9')
        return true;

    if (c >= 'a' && c <= 'f')
        return true;

    if (c >= 'A' && c <= 'F')
        return true;

    return false;
}

static auto hexDigitToChar(char c) -> unsigned char {
    // 0-9
    if (c >= '0' && c <= '9')
        return c - 48;

    // a-f
    if (c >= 'a' && c <= 'f')
        return c - 87;

    // A-F
    if (c >= 'A' && c <= 'F')
        return c - 55;

    return 0;
}

auto up::UUID::fromString(string_view id) noexcept -> UUID {
    auto len = id.size();
    if (len != 36)
        return UUID::zero();

    char buffer[2]; // reading 2 chars at a time
    uint32 chidx = 0;

    UUID result;
    uint32 bidx = 0;
    for (auto c : id) {
        if (c == '-')
            continue;

        if (!isValidChar(c))
            return UUID::zero();

        buffer[chidx++] = c;

        if (chidx == 2) {
            byte v = static_cast<byte>(hexDigitToChar(buffer[0]) * 16 + hexDigitToChar(buffer[1]));
            chidx = 0;
            result._data.ub[bidx++] = v;
        }
    }

    return result;
}
