// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "task_system.h"
#include "thread.h"
#include <grimm/foundation/string_blob.h>
#include <grimm/foundation/string_writer.h>
#include <grimm/foundation/string_format.h>
#include <thread>

gm::Tasks::~Tasks() {
    // signal all workers to close
    _queue->close();

    // this will wait for all the workers to complete
    _workers.clear();
}

void gm::Tasks::createWorkers(int numWorkers) {
    TaskQueue& queue = *_queue;

    _workers.reserve(numWorkers);
    string_writer writer;
    for (int n = int(_workers.size()); n < numWorkers; ++n) {
        writer.clear();
        format_into(writer, "Task Worker [{}]", n);
        _workers.emplace_back(queue, writer.to_string());
    }
}

int gm::Tasks::getHardwareConcurrencyLevel() const {
    return std::thread::hardware_concurrency();
}

bool gm::Tasks::work() {
    if (_semaphore.tryWait()) {
        rc<Task> task;
        if (_queue->tryDeque(task)) {
            task->execute();
            return true;
        }
    }

    // no work was available
    return false;
}

void gm::Tasks::workOrBlock() {
    if (!isClosed()) {
        _semaphore.wait();

        rc<Task> task;
        if (_queue->tryDeque(task)) {
            task->execute();
        }
    }
}
