// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <grimm/foundation/box.h>
#include <grimm/foundation/types.h>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace gm {
    template <typename T, uint32 Size = 1024>
    class ConcurrentQueue {
        static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");

    public:
        ConcurrentQueue();
        ~ConcurrentQueue();

        ConcurrentQueue(ConcurrentQueue const&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue const&) = delete;

        void close();
        void waitUntilEmpty();

        template <typename InsertT>
        [[nodiscard]] bool tryEnque(InsertT&& value);
        [[nodiscard]] bool tryDeque(T& out);

        template <typename InsertT>
        void enqueWait(InsertT&& value);
        [[nodiscard]] bool dequeWait(T& out);

    private:
        void _grow();

        using storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
        static constexpr uint32 mask = Size - 1;

        std::mutex _lock;
        std::condition_variable _condition;
        bool _closed = false;
        uint32 _start = 0;
        uint32 _size = 0;
        storage* _buffer = nullptr;
    };

    template <typename T, uint32 Size>
    ConcurrentQueue<T, Size>::ConcurrentQueue() : _buffer(new storage[Size]) {}

    template <typename T, uint32 Size>
    ConcurrentQueue<T, Size>::~ConcurrentQueue() {
        close();
        waitUntilEmpty();
        delete[] _buffer;
    }

    template <typename T, uint32 Size>
    void ConcurrentQueue<T, Size>::waitUntilEmpty() {
        for (;;) {
            {
                std::unique_lock lock(_lock);

                if (_size == 0) {
                    return;
                }
            }

            std::this_thread::yield();
        }
    }

    template <typename T, uint32 Size>
    template <typename InsertT>
    bool ConcurrentQueue<T, Size>::tryEnque(InsertT&& value) {
        std::unique_lock lock(_lock);

        if (_closed) {
            return false;
        }

        if (_size == mask + 1) {
            return false;
        }

        new (&_buffer[(_start + _size) & mask]) T(std::forward<InsertT>(value));
        ++_size;

        _condition.notify_one();
        return true;
    }

    template <typename T, uint32 Size>
    bool ConcurrentQueue<T, Size>::tryDeque(T& out) {
        std::unique_lock lock(_lock);

        if (_size != 0) {
            out = reinterpret_cast<T&&>(_buffer[(_start + _size - 1) & mask]);
            ++_start;
            --_size;
            return true;
        }

        return false;
    }

    template <typename T, uint32 Size>
    template <typename InsertT>
    void ConcurrentQueue<T, Size>::enqueWait(InsertT&& value) {
        while (!tryEnque(std::forward<InsertT>(value))) {
            std::this_thread::yield();
        }
    }

    template <typename T, uint32 Size>
    bool ConcurrentQueue<T, Size>::dequeWait(T& out) {
        std::unique_lock lock(_lock);
        for (;;) {
            _condition.wait(lock, [this] { return _size != 0 || _closed; });

            // check for spurious wakeup
            if (_size != 0) {
                out = reinterpret_cast<T&&>(_buffer[(_start + _size - 1) & mask]);
                ++_start;
                --_size;
                return true;
            }

            if (_closed) {
                return false;
            }
        }
    }

    template <typename T, uint32 Size>
    void ConcurrentQueue<T, Size>::close() {
        std::unique_lock lock(_lock);

        _closed = true;
        _condition.notify_all();
    }

} // namespace gm
