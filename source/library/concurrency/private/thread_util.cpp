// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "grimm/concurrency/thread_util.h"
#include <grimm/foundation/assertion.h>
#include <atomic>
#include <limits>

auto gm::concurrency::currentSmallThreadId() noexcept -> SmallThreadId {
    static std::atomic<SmallThreadId> nextThreadId = 0;
    static thread_local SmallThreadId currentThreadId = nextThreadId++;

    GM_ASSERT(currentThreadId < std::numeric_limits<SmallThreadId>::max(), "Too many threads have been created using currentThreadSmallId");

    return currentThreadId;
}
