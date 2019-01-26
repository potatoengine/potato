// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "task.h"
#include "lock_free_queue.h"
#include "fast_semaphore.h"
#include <grimm/foundation/box.h>
#include <grimm/foundation/rc.h>
#include <grimm/foundation/vector.h>
#include <atomic>

namespace gm {

    class Task;

    class TaskQueue {
    public:
        GM_CONCURRENCY_API TaskQueue();
        GM_CONCURRENCY_API ~TaskQueue();

        TaskQueue(TaskQueue const&) = delete;
        TaskQueue& operator=(TaskQueue const&) = delete;

        static GM_CONCURRENCY_API TaskQueue& getInstance();

        GM_CONCURRENCY_API void close();
        bool isClosed() const { return _closed.load(std::memory_order_relaxed); }

        GM_CONCURRENCY_API void enque(rc<Task> task);

        GM_CONCURRENCY_API void await(Task const& task);

        GM_CONCURRENCY_API bool work();
        GM_CONCURRENCY_API void workOrBlock();

    private:
        box<LockFreeQueue<rc<Task>>> _queue;
        Semaphore _semaphore;
        std::atomic_bool _closed = false;

        static box<TaskQueue> _instance;
    };

} // namespace gm
