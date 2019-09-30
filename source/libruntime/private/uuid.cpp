// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/spud/string_writer.h"

using namespace up;


uuid::uuid() noexcept {
    std::memset(&_data, 0, sizeof(_data));
}

uuid::uuid(const uuid& other) noexcept {
    if (this == &other)
        return;
    std::memcpy(&_data, &other._data, sizeof(_data));
}

uuid::uuid(const buffer& value) noexcept {
    std::memcpy(_data.data(), value.data(), sizeof(_data));
}

auto uuid::isValid() noexcept -> bool {
    return *this != uuid::zero();
}

uuid uuid::generate() noexcept {
    return uuid(_generate());
}

uuid uuid::zero() noexcept {
    static uuid _zero(uuid::buffer({}));
    return _zero;
}

constexpr char byteToString(up::byte byte) noexcept {
    return static_cast<char>(byte);
}

string uuid::toString(const uuid& id) {
    // format 9554084e-4100-4098-b470-2125f5eed133
    string_writer buffer;
    format_into(buffer, "{:02x}{:02x}{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}-{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
                id._data[0], id._data[1], id._data[2], id._data[3],
                id._data[4], id._data[5],
                id._data[6], id._data[7],
                id._data[8], id._data[9],
                id._data[10], id._data[11], id._data[12], id._data[13], id._data[14], id._data[15]);

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

uuid uuid::fromString(string_view id) noexcept {
    auto len = id.size();
    if (len != 36)
        return uuid::zero();

    char buffer[2]; // reading 2 chars at a time
    uint32 chidx = 0;

    uuid result;
    uint32 bidx = 0;
    for (auto c : id) {
        if (c == '-')
            continue;

        if (!isValidChar(c))
            return uuid::zero();

        buffer[chidx++] = c;

        if (chidx == 2) {
            byte v = static_cast<byte>(hexDigitToChar(buffer[0]) * 16 + hexDigitToChar(buffer[1]));
            chidx = 0;
            result._data[bidx++] = v;
        }
    }

    return result;
}
