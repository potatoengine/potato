// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"

using namespace up;

#ifdef UP_PLATFORM_WINDOWS
#    include "potato/foundation/platform_windows.h"
#    include <rpc.h>
#elif defined UP_PLATFORM_LINUX
#    include <uuid/uuid.h>
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
    std::memcpy(ret.data(), &bytes, sizeof(UUID));
#endif

    return ret;
}

uuid uuid::generate() {

    uuid::buffer temp = generateGuid();
    return uuid(temp);
}

uuid uuid::zero() {
    static uuid _zero({});
    return _zero;
}

string uuid::toString(const uuid& id) {

#ifdef UP_PLATFORM_WINDOWS
    RPC_CSTR str;
    const UUID* data = (const UUID*)id._data.data();
    if (UuidToStringA(data, &str) != RPC_S_OK)
        return "";

    string s((const char*)str);
    RpcStringFreeA(&str);
    return s;

#else
    char s[37];
    uuid_unparse((unsigned char*)id._data.data(), s);
    return s;
#endif
}
