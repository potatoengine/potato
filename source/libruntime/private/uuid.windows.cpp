// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/assertion.h"
#include "potato/spud/platform_windows.h"
#include <rpc.h>

#if !defined(UP_PLATFORM_WINDOWS)
#    error "Unsupported platform"
#endif

auto up::uuid::_generate() noexcept -> uuid::buffer {
    UUID temp;
    if (RPC_S_OK != UuidCreate(&temp)) {
        UP_ASSERT(false, "Failed to generate unique ID.");
    }

    uuid::buffer ret;
    static_assert(sizeof(ret) == sizeof(UUID));
    std::memcpy(ret.data(), &temp, sizeof(UUID));
    return ret;
}
