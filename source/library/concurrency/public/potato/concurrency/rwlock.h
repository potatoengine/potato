// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <atomic>

namespace up::concurrency {
    class RWLock {
    public:
        class Reader {
        public:
            explicit Reader(std::atomic<unsigned>& lock) noexcept : _lock(lock) {}

            inline void lock() noexcept;
            inline bool tryLock() noexcept;
            inline void unlock() noexcept;

        private:
            std::atomic<unsigned>& _lock;
        };

        class Writer {
        public:
            explicit Writer(std::atomic<unsigned>& lock) noexcept : _lock(lock) {}

            inline void lock() noexcept;
            inline bool tryLock() noexcept;
            inline void unlock() noexcept;

        private:
            std::atomic<unsigned>& _lock;
        };

        RWLock() noexcept : _lock(0), _reader(_lock), _writer(_lock) {}

        Reader& reader() noexcept { return _reader; }
        Writer& writer() noexcept { return _writer; }

    private:
        std::atomic<unsigned> _lock;
        Reader _reader;
        Writer _writer;
    };

    void RWLock::Reader::lock() noexcept {
        // FIXME - exponential backoff should be added
        unsigned expected = _lock.load(std::memory_order_relaxed);
        while (expected == unsigned(-1) || _lock.compare_exchange_weak(expected, expected + 1, std::memory_order_acquire)) {
        }
    }

    bool RWLock::Reader::tryLock() noexcept {
        unsigned expected = _lock.load(std::memory_order_relaxed);
        return expected != unsigned(-1) && _lock.compare_exchange_strong(expected, expected + 1, std::memory_order_acquire);
    }

    void RWLock::Reader::unlock() noexcept {
        _lock.fetch_sub(1, std::memory_order_release);
    }

    void RWLock::Writer::lock() noexcept {
        // FIXME - exponential backoff should be added
        unsigned expected = 0;
        while (_lock.compare_exchange_weak(expected, unsigned(-1), std::memory_order_acquire)) {
        }
    }

    bool RWLock::Writer::tryLock() noexcept {
        unsigned expected = 0;
        return _lock.compare_exchange_strong(expected, unsigned(-1), std::memory_order_acquire);
    }

    void RWLock::Writer::unlock() noexcept {
        _lock.store(0, std::memory_order_release);
    }

} // namespace up::concurrency
