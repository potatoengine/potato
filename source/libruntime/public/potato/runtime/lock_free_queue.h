// Copyright (C) 2014-2016,2019 Sean Middleditch, all rights reserverd.

// http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue

#pragma once

#include <type_traits>
#include <atomic>
#include <cstdint>

namespace up {
    template <typename T, std::size_t CacheLineWidth = 64>
    struct alignas(CacheLineWidth) AlignedAtomic : std::atomic<T> {
        char _padding[CacheLineWidth - sizeof(std::atomic<T>)];
    };

    template <typename T, std::size_t Size = 512, std::size_t CacheLineWidth = 64>
    class LockFreeQueue {
        static constexpr std::uint32_t kBufferSize = Size;
        static constexpr std::uint32_t kBufferMask = kBufferSize - 1;

        static_assert((kBufferSize & kBufferMask) == 0, "ConcurrentQueue size must be a power of 2");

        AlignedAtomic<std::uint32_t, CacheLineWidth> _sequence[kBufferSize];
        AlignedAtomic<std::uint32_t, CacheLineWidth> _enque;
        AlignedAtomic<std::uint32_t, CacheLineWidth> _deque;
        std::aligned_storage_t<sizeof(T), alignof(T)> _buffer[kBufferSize];

    public:
        inline LockFreeQueue();
        LockFreeQueue(LockFreeQueue const&) = delete;
        LockFreeQueue& operator=(LockFreeQueue const&) = delete;

        template <typename InsertT>
        [[nodiscard]] inline bool tryEnque(InsertT&& value);
        [[nodiscard]] inline bool tryDeque(T& out);
    };

    template <typename T, std::size_t Size, std::size_t CacheLineWidth>
    LockFreeQueue<T, Size, CacheLineWidth>::LockFreeQueue() {
        _enque.store(0, std::memory_order_relaxed);
        _deque.store(0, std::memory_order_relaxed);
        for (std::uint32_t i = 0; i != kBufferSize; ++i)
            _sequence[i].store(i, std::memory_order_relaxed);
    }

    template <typename T, std::size_t Size, std::size_t CacheLineWidth>
    template <typename InsertT>
    bool LockFreeQueue<T, Size, CacheLineWidth>::tryEnque(InsertT&& value) {
        std::uint32_t target = _enque.load(std::memory_order_relaxed);
        std::uint32_t id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
        std::int32_t delta = id - target;

        while (!(delta == 0 && _enque.compare_exchange_weak(target, target + 1, std::memory_order_relaxed))) {
            if (delta < 0) {
                return false;
            }

            target = _enque.load(std::memory_order_relaxed);
            id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
            delta = id - target;
        }

        new (&_buffer[target & kBufferMask]) T(std::forward<InsertT>(value));
        _sequence[target & kBufferMask].store(target + 1, std::memory_order_release);
        return true;
    }

    template <typename T, std::size_t Size, std::size_t CacheLineWidth>
    bool LockFreeQueue<T, Size, CacheLineWidth>::tryDeque(T& out) {
        std::uint32_t target = _deque.load(std::memory_order_relaxed);
        std::uint32_t id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
        std::int32_t delta = id - (target + 1);

        while (!(delta == 0 && _deque.compare_exchange_weak(target, target + 1, std::memory_order_relaxed))) {
            if (delta < 0) {
                return false;
            }

            target = _deque.load(std::memory_order_relaxed);
            id = _sequence[target & kBufferMask].load(std::memory_order_acquire);
            delta = id - (target + 1);
        }

        out = std::move(reinterpret_cast<T&>(_buffer[target & kBufferMask]));
        reinterpret_cast<T&>(_buffer[target & kBufferMask]).~T();
        _sequence[target & kBufferMask].store(target + kBufferMask + 1, std::memory_order_release);
        return true;
    }

} // namespace up
