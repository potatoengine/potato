// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/foundation/uuid.h"
#include "potato/foundation/assertion.h"

using namespace up;

uuid uuid::ZERO = uuid({0});

uuid::uuid() noexcept {
    std::memset(&_value, 0, sizeof(_value));
}

uuid::uuid(const uuid& other) noexcept {
    if (this == &other)
        return;
    std::memcpy(&_value, &other._value, sizeof(_value));
}

uuid::uuid(const uuid_t& other) noexcept {
    std::memcpy(&_value, &other, sizeof(_value));
}

auto uuid::isValid() noexcept -> bool {
    return *this != uuid::ZERO;
}

uuid uuid::generate() {

#ifdef WIN32
    UUID temp;
    if (RPC_S_OK != UuidCreate(&temp))
        UP_ASSERT(false, "Failed to generate unique ID.");
#else
    uuid_t temp;
    uuid_generate_random(temp);
#endif

    return uuid(temp);
}

string uuid::toString(const uuid& id) {

#ifdef WIN32
    RPC_CSTR str;
    if (UuidToStringA(&id._value, &str) != RPC_S_OK)
        return "";

    string s((const char*)str);
    RpcStringFreeA(&str);
    return s;

#else
    uuid_t uuid;
    uuid_generate_random(uuid);
    char s[37];
    uuid_unparse(uuid, s);
#endif
}
