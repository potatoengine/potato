// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/spud/platform.h"
#include <CoreFoundation/CFUUID.h>

#if !defined(UP_PLATFORM_APPLE)
#    error "Unsupported platform"
#endif

auto up::uuid::_generate() noexcept -> uuid::buffer {

    auto newId = CFUUIDCreate(NULL);
    auto bytes = CFUUIDGetUUIDBytes(newId);
    CFRelease(newId);

    uuid::buffer ret;
    static_assert(sizeof(ret) == sizeof(bytes));
    std::memcpy(ret.data(), &bytes, sizeof(bytes));
    return ret;
}
