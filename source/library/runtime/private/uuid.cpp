// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/foundation/string_writer.h"

using namespace up;

#ifdef UP_PLATFORM_WINDOWS
#    include "potato/foundation/platform_windows.h"
#    include <rpc.h>
#elif defined UP_PLATFORM_LINUX
#    include <uuid/uuid.h>
#elif defined UP_PLATFORM_APPLE
#    include <CoreFoundation/CFUUID.h>
#else
#    error "Unsupported platform"
#endif

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
auto generateGuid() -> uuid::buffer {
    uuid::buffer ret;
#ifdef UP_PLATFORM_WINDOWS
    UUID temp;
    if (RPC_S_OK != UuidCreate(&temp))
        UP_ASSERT(false, "Failed to generate unique ID.");

    static_assert(sizeof(ret) == sizeof(UUID));
    std::memcpy(ret.data(), &temp, sizeof(UUID));
#elif UP_PLATFORM_LINUX
    static_assert(sizeof(ret) == sizeof(uuid));
    uuid_generate((unsigned char*)ret.data());
#elif UP_PLATFORM_APPLE
    auto newId = CFUUIDCreate(NULL);
    auto bytes = CFUUIDGetUUIDBytes(newId);
    CFRelease(newId);
    static_assert(sizeof(ret) == sizeof(bytes));
    std::memcpy(ret.data(), &bytes, sizeof(bytes));
#endif

    return ret;
}

uuid uuid::generate() {
    uuid::buffer temp = generateGuid();
    return uuid(temp);
}

uuid uuid::zero() {
    static uuid _zero(uuid::buffer({}));
    return _zero;
}

char byteToString(up::byte& byte) {
    return (char)byte;
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

uuid uuid::fromString(string_view id) {
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
