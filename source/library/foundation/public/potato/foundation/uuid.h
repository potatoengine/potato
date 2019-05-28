// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string.h"

#ifdef UP_PLATFORM_WINDOWS
#    include <windows.h>
#elif defined UP_PLATFORM_LINUX
#    include <uuid/uuid.h>
#else
#    error "Unsupported platform"
#endif

namespace up {

    class UP_FOUNDATION_API uuid {
    public:
        uuid() noexcept;
        uuid(const uuid& other) noexcept;
        uuid(const uuid_t& other) noexcept;

        bool isValid() noexcept;

        bool operator==(const uuid& other) const { return std::memcmp(&_value, &other._value, sizeof(_value)) == 0; }
        bool operator!=(const uuid& other) const { return !operator==(other); }

        static uuid generate();
        static string toString(const uuid& id);

        static uuid zero();

        uuid_t _value;
    };

} // namespace up
