// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "task_queue.h"
#include "lock_free_queue.h"
#include "semaphore.h"
#include "thread.h"

gm::box<gm::TaskQueue> gm::TaskQueue::_instance;

gm::TaskQueue::TaskQueue() : _queue(make_box<LockFreeQueue<rc<Task>>>()) {
}

gm::TaskQueue::~TaskQueue() {
    close();
}

auto gm::TaskQueue::getInstance() -> TaskQueue& {
    if (!_instance)
        _instance = make_box<TaskQueue>();
    return *_instance;
}

void gm::TaskQueue::enque(rc<Task> task) {
    if (!isClosed()) {
        task->schedule();
        while (!_queue->tryEnque(std::move(task)))
            if (!work())
                Thread::yieldTimeSlice();

        _semaphore.signal();
    }
}

void gm::TaskQueue::await(Task const& task) {
    while (!task.isReady()) {
        if (!work())
            Thread::yieldTimeSlice();
    }
}

bool gm::TaskQueue::work() {
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

void gm::TaskQueue::close() {
    _closed.store(true, std::memory_order_seq_cst);
    _semaphore.signal(std::numeric_limits<int>::max());
}

void gm::TaskQueue::workOrBlock() {
    if (!isClosed()) {
        _semaphore.wait();

        rc<Task> task;
        if (_queue->tryDeque(task)) {
            task->execute();
        }
    }
}
