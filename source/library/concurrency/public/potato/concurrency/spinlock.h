// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/foundation/assertion.h"
#include <atomic>
#include <thread>

namespace up::concurrency {
    class Spinlock {
    public:
        inline void lock() noexcept;
        inline [[nodiscard]] bool tryLock() noexcept;
        inline [[nodiscard]] bool isLocked() const noexcept;
        inline void unlock() noexcept;

    private:
        std::atomic<std::thread::id> _owner = std::thread::id();
    };

    void Spinlock::lock() noexcept  {
        // try to acquire the lock
        // FIXME - exponential backoff should be added
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        while (!_owner.compare_exchange_weak(expected, desired, std::memory_order_acquire)) {
            expected = std::thread::id();
        }
    }

    bool Spinlock::tryLock() noexcept  {
        // try to acquire the lock
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        return _owner.compare_exchange_strong(expected, desired, std::memory_order_acquire);
    }

    bool Spinlock::isLocked() const noexcept  {
        return _owner != std::thread::id();
    }

    void Spinlock::unlock() noexcept  {
        UP_ASSERT(_owner == std::this_thread::get_id());

        // release the lock
        _owner.store(std::thread::id(), std::memory_order_release);
    }

} // namespace up::concurrency
