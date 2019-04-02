// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <grimm/foundation/assertion.h>
#include <atomic>
#include <thread>

namespace gm::concurrency {
    class Spinlock {
    public:
        inline void lock();
        inline [[nodiscard]] bool tryLock();
        inline [[nodiscard]] bool isLocked() const;
        inline void unlock();

    private:
        std::atomic<std::thread::id> _owner = std::thread::id();
    };

    class SpinlockGuard {
        Spinlock& _lock;

    public:
        SpinlockGuard(Spinlock& lock) : _lock(lock) { _lock.lock(); }
        ~SpinlockGuard() { _lock.unlock(); }

        SpinlockGuard(SpinlockGuard const&) = delete;
        SpinlockGuard& operator=(SpinlockGuard const&) = delete;
    };

    void Spinlock::lock() {
        // try to acquire the lock
        // FIXME - exponential backoff should be added
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        while (!_owner.compare_exchange_weak(expected, desired, std::memory_order_acquire)) {
            expected = std::thread::id();
        }
    }

    bool Spinlock::tryLock() {
        // try to acquire the lock
        std::thread::id expected{};
        auto const desired = std::this_thread::get_id();
        return _owner.compare_exchange_strong(expected, desired, std::memory_order_acquire);
    }

    bool Spinlock::isLocked() const {
        return _owner != std::thread::id();
    }

    void Spinlock::unlock() {
        GM_ASSERT(_owner == std::this_thread::get_id());

        // release the lock
        _owner.store(std::thread::id(), std::memory_order_release);
    }

} // namespace gm::concurrency
