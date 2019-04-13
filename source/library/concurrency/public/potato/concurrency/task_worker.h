// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "thread_util.h"
#include "concurrent_queue.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/delegate.h"
#include <thread>

namespace up {
    using Task = delegate<void()>;
    using TaskQueue = ConcurrentQueue<Task>;

    class TaskWorker {
    public:
        UP_CONCURRENCY_API explicit TaskWorker(TaskQueue& queue, zstring_view name);
        UP_CONCURRENCY_API ~TaskWorker();

        TaskWorker(TaskWorker&&) = default;
        TaskWorker& operator=(TaskWorker&&) = default;

        SmallThreadId smallThreadId() const noexcept { return _threadId; }

        void detach() { _thread.detach(); }
        void join() { _thread.join(); }

    private:
        int _threadMain();

        TaskQueue& _queue;
        SmallThreadId _threadId;
        std::thread _thread;
    };

} // namespace up
