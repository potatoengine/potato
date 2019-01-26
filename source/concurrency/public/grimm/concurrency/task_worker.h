// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "thread.h"
#include <grimm/foundation/string_blob.h>

namespace gm {

    class TaskQueue;

    class TaskWorker {
    public:
        GM_CONCURRENCY_API explicit TaskWorker(TaskQueue& queue, string name);
        GM_CONCURRENCY_API ~TaskWorker();

        TaskWorker(TaskWorker&&) = default;
        TaskWorker& operator=(TaskWorker&&) = default;

    private:
        int _threadMain();

        TaskQueue& _queue;
        Thread _thread;
    };

} // namespace gm
