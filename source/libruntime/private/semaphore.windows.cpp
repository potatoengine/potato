// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

// Inspired by the technique in the "LightweightSemaphore" at
//   https://github.com/preshing/cpp11-on-multicore

#include "potato/runtime/semaphore.h"
#include "potato/spud/platform_windows.h"
#include <limits>

up::Semaphore::Semaphore(int initial) noexcept  : _counter(initial), _handle(CreateSemaphoreW(nullptr, 0, std::numeric_limits<LONG>::max(), nullptr)) {
}

up::Semaphore::~Semaphore() noexcept  {
    CloseHandle(_handle);
}

void up::Semaphore::_signal(int n) noexcept {
    ReleaseSemaphore(_handle, n, nullptr);
}

void up::Semaphore::_wait() noexcept {
    constexpr int kMaxSpin = 10000;

    // keep spinning for a while since OS sychronization is expensive
    int spin = kMaxSpin;
    while (--spin != 0) {
        // if we have at least one pending item, try to consume it
        if (tryWait()) {
            return;
        }

        // suppress the loop from being optimized away even though we don't use `spin` for anything
        std::atomic_signal_fence(std::memory_order_acquire);
    }

    // spinlock has been exhausted trying to consume an available (e.g., decrementing a positive values).
    // we will now "consume" the item whether it's available or not. if there was no available item (e.g.,
    // the counter was non-positive) then we block.
    if (_counter.fetch_sub(1, std::memory_order_acquire) <= 0) {
        WaitForSingleObject(_handle, INFINITE);
    }
}
