// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "concurrent_queue.h"
#include "task_worker.h"
#include <grimm/foundation/vector.h>
#include <grimm/foundation/box.h>

namespace gm {

    class Tasks {
    public:
        Tasks() = default;
        GM_CONCURRENCY_API ~Tasks();

        Tasks(Tasks const&) = delete;
        Tasks& operator=(Tasks const&) = delete;

        GM_CONCURRENCY_API void createWorkers(int numWorkers);

        GM_CONCURRENCY_API int getHardwareConcurrencyLevel() const;

        GM_CONCURRENCY_API bool work();
        GM_CONCURRENCY_API void workOrBlock();

        TaskQueue& queue() noexcept { return *_queue; }

    private: // members
        box<TaskQueue> _queue;
        vector<TaskWorker> _workers;
    };

} // namespace gm
