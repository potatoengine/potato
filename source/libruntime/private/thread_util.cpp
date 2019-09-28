// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/thread_util.h"
#include <potato/runtime/assertion.h>
#include <atomic>
#include <limits>

auto up::currentSmallThreadId() noexcept -> SmallThreadId {
    static std::atomic<SmallThreadId> nextThreadId = 0;
    static thread_local SmallThreadId currentThreadId = nextThreadId++;

    UP_ASSERT(currentThreadId < std::numeric_limits<SmallThreadId>::max(), "Too many threads have been created using currentThreadSmallId");

    return currentThreadId;
}