// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "potato/concurrency/thread_util.h"
#include "potato/foundation/assertion.h"
#include <atomic>
#include <limits>

auto up::concurrency::currentSmallThreadId() noexcept -> SmallThreadId {
    static std::atomic<SmallThreadId> nextThreadId = 0;
    static thread_local SmallThreadId currentThreadId = nextThreadId++;

    UP_ASSERT(currentThreadId < std::numeric_limits<SmallThreadId>::max(), "Too many threads have been created using currentThreadSmallId");

    return currentThreadId;
}
