// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

// Inspired by the technique in the "LightweightSemaphore" at
//   https://github.com/preshing/cpp11-on-multicore

#include "semaphore.h"
#include <grimm/foundation/platform_windows.h>
#include <limits>

gm::Semaphore::Semaphore(int initial) : _handle(CreateSemaphoreW(nullptr, initial, std::numeric_limits<LONG>::max(), nullptr)) {
}

gm::Semaphore::~Semaphore() {
    CloseHandle(_handle);
}

void gm::Semaphore::_signal(int n) {
    ReleaseSemaphore(_handle, n, nullptr);
}

void gm::Semaphore::_wait() {
    constexpr int kMaxSpin = 10000;

    // keep spinning for a while since OS sychronization is expensive
    int spin = kMaxSpin;
    while (--spin != 0) {
        // if we have at least one pending item, try to consume it
        if (tryWait())
            return;

        // suppress the loop from being optimized away even though we don't use `spin` for anything
        std::atomic_signal_fence(std::memory_order_acquire);
    }

    // spinlock has been exhausted trying to consume an available (e.g., decrementing a positive values).
    // we will now "consume" the item whether it's available or not. if there was no available item (e.g.,
    // the counter was non-positive) then we block.
    if (_counter.fetch_sub(1, std::memory_order_acquire) <= 0)
        WaitForSingleObject(_handle, INFINITE);
}

int gm::Semaphore::wait(Semaphore& sema1, Semaphore& sema2) {
    constexpr int kMaxSpin = 10000;

    int spin = kMaxSpin;
    while (--spin != 0) {
        if (sema1.tryWait())
            return 0;
        else if (sema2.tryWait())
            return 1;

        std::atomic_signal_fence(std::memory_order_acquire);
    }

    // see comments on _wait for this logic - note the inverted logic here, so the
    // <=0 in the _wait function becomes >0 in this function (e.g., return if there
    // was an available item, because the return value was positive).
    if (sema1._counter.fetch_sub(1, std::memory_order_acquire) > 0)
        return 0;
    if (sema2._counter.fetch_sub(1, std::memory_order_acquire) > 0)
        return 1;

    HANDLE handles[] = {sema1._handle, sema2._handle};
    DWORD const result = WaitForMultipleObjects(static_cast<DWORD>(std::size(handles)), std::data(handles), false, INFINITE);
    return result;
}
