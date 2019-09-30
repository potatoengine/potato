// Copyright (C) 2014 Sean Middleditch, all rights reserverd.

#include "potato/runtime/uuid.h"
#include "potato/runtime/platform.h"

#include <uuid/uuid.h>

#if !defined(UP_PLATFORM_LINUX)
#    error "Unsupported platform"
#endif

auto up::uuid::_generate() noexcept -> uuid::buffer {
    uuid::buffer ret;
    uuid_generate((unsigned char*)ret.data());
    return ret;
}
