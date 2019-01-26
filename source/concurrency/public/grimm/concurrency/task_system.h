// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "task_queue.h"
#include "task_worker.h"
#include <grimm/foundation/vector.h>

namespace gm {

    class Tasks {
    public:
        Tasks() = default;
        GM_CONCURRENCY_API ~Tasks();

        Tasks(Tasks const&) = delete;
        Tasks& operator=(Tasks const&) = delete;

        GM_CONCURRENCY_API void createWorkers(int numWorkers);

        GM_CONCURRENCY_API int getHardwareConcurrencyLevel() const;

    private: // members
        vector<TaskWorker> _workers;
    };

} // namespace gm
