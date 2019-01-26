// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

// Inspired by the technique in the "LightweightSemaphore" at
//   https://github.com/preshing/cpp11-on-multicore

#include "fast_semaphore.h"
#include <limits>
#include <mutex>
#include <condition_variable>

namespace {
    struct SemaphoreImplementation {
        std::mutex lock;
        std::condition_variable cond;
        int count = 0;
    };
} // namespace

gm::Semaphore::Semaphore(int initial) {
    SemaphoreImplementation* sema = new SemaphoreImplementation;
    sema->count = initial;
    _handle = sema;
}

gm::Semaphore::~Semaphore() {
    delete _handle;
}

void gm::Semaphore::_signal(int n) {
    auto sema = static_cast<SemaphoreImplementation*>(_handle);
    std::lock_guard lock(sema->lock);

    sema->count += n;

    if (n == 1) {
        sema->cond.notify_one();
    }
    else {
        sema->cond.notify_all();
    }
}

void gm::Semaphore::_wait() {
    auto sema = static_cast<SemaphoreImplementation*>(_handle);
    std::lock_guard lock(sema->lock);

    sema->cond.wait(sema->lock, [sema] {
        return count > 0;
    });
}
