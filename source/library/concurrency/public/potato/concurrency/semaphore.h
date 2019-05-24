// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/span.h"
#include "potato/foundation/platform.h"
#include <atomic>

namespace up {
    class Semaphore {
    public:
        UP_CONCURRENCY_API Semaphore(int initial = 0);
        UP_CONCURRENCY_API ~Semaphore();

        Semaphore(Semaphore const&) = delete;
        Semaphore& operator=(Semaphore const&) = delete;

        UP_FORCEINLINE inline void signal(int count = 1);
        [[nodiscard]] UP_FORCEINLINE inline bool tryWait();
        UP_FORCEINLINE inline void wait();

    private:
        UP_CONCURRENCY_API void _signal(int n);
        UP_CONCURRENCY_API void _wait();

        std::atomic<int> _counter = 0;
        void* _handle = nullptr;
    };

    void Semaphore::signal(int count) {
        // add an item (increase the counter) and record how many waiting threads we had;
        // the count goes negative for waiting threads, so the number of waiting threads
        // is the negative of the old count
        int const waiting = -_counter.fetch_add(count, std::memory_order_release);

        // awaken up to `count` of the waiting threads
        int const awaken = /*min(waiting,count)*/ waiting < count ? waiting : count;
        if (awaken > 0) {
            _signal(awaken);
        }
    }

    bool Semaphore::tryWait() {
        // consume a single item if there are any available (positive count)
        int count = _counter.load(std::memory_order_relaxed);
        return count > 0 && _counter.compare_exchange_strong(count, count - 1, std::memory_order_acquire);
    }

    void Semaphore::wait() {
        if (!tryWait()) {
            _wait();
        }
    }

} // namespace up
