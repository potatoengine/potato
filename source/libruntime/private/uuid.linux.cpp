// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/spud/platform.h"
#include <uuid/uuid.h>

#if !defined(UP_PLATFORM_LINUX)
#    error "Unsupported platform"
#endif

auto up::uuid::generate() noexcept -> uuid {
    uuid ret;
    uuid_generate(static_cast<char*>(ret._data.ub));
    return ret;
}
