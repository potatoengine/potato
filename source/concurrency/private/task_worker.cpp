// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "task_worker.h"
#include "task_queue.h"

gm::TaskWorker::TaskWorker(TaskQueue& queue, string name) : _queue(queue) {
    _thread = Thread::spawn([this] { return _threadMain(); }, std::move(name));
}

gm::TaskWorker::~TaskWorker() {
    _thread.joinThread();
}

int gm::TaskWorker::_threadMain() {
    while (!_queue.isClosed())
        _queue.workOrBlock();

    return 0;
}
