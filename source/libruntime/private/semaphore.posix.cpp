// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

// Inspired by the technique in the "LightweightSemaphore" at
//   https://github.com/preshing/cpp11-on-multicore

#include "semaphore.h"

#include <condition_variable>
#include <limits>
#include <mutex>

namespace {
    struct SemaphoreImplementation {
        std::mutex lock;
        std::condition_variable cond;

        // separately tracked from the semaphore counter,
        // because this is behind the same lock as the
        // condition_variable which avoids race conditions -
        // the negative value of the counter tracks how
        // many waiters there _will_ be but doesn't denote
        // how many are actually in the condition_variable's
        // wait loop - this variable does
        int waiters = 0;
    };
} // namespace

up::Semaphore::Semaphore(int initial) noexcept : _counter(initial), _handle(new SemaphoreImplementation) {}

up::Semaphore::~Semaphore() noexcept {
    delete static_cast<SemaphoreImplementation*>(_handle);
}

void up::Semaphore::_signal(int n) noexcept {
    auto sema = static_cast<SemaphoreImplementation*>(_handle);

    {
        std::unique_lock lock(sema->lock);
        sema->waiters -= n;
    }

    if (n == 1) {
        sema->cond.notify_one();
    }
    else {
        sema->cond.notify_all();
    }
}

void up::Semaphore::_wait() noexcept {
    if (_counter.fetch_sub(1, std::memory_order_acquire) <= 0) {
        auto sema = static_cast<SemaphoreImplementation*>(_handle);
        std::unique_lock lock(sema->lock);

        ++sema->waiters;
        for (;;) {
            sema->cond.wait(lock, [sema] { return sema->waiters <= 0; });

            if (sema->waiters <= 0) {
                return;
            }
        }
    }
}
