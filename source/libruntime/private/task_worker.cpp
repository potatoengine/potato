// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/task_worker.h"
#include "potato/runtime/semaphore.h"
#include "potato/runtime/thread_util.h"

up::TaskWorker::TaskWorker(ConcurrentQueue<Task>& queue, zstring_view name) : _queue(queue) {
    // just to make sure this is called at least once on the main thread...
    [[maybe_unused]] auto const _ = currentSmallThreadId();

    Semaphore sem;
    _thread = std::thread([this, name, &sem] {
        setCurrentThreadName(name);
        _threadId = currentSmallThreadId();
        sem.signal();
        return _threadMain();
    });

    // ensure the thread starts and sets its name and our thread-id
    // we want the threadId to be accurate by the end of construction,
    // and the backing of name may go out of scope afterward
    sem.wait();
}

up::TaskWorker::~TaskWorker() {
    if (_thread.joinable()) {
        _thread.join();
    }
}

int up::TaskWorker::_threadMain() {
    Task task;
    while (_queue.dequeWait(task)) {
        task();
    }
    return 0;
}
