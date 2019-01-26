// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "thread.h"
#include <grimm/foundation/delegate.h>
#include <grimm/foundation/string_view.h>
#include <grimm/foundation/types.h>
#include <atomic>

static std::atomic<gm::uint64> nextThreadId = 1;
static thread_local gm::ThreadId currentThreadId = gm::ThreadId::None;

auto gm::Thread::getCurrentThreadId() -> ThreadId {
    return currentThreadId;
}

void gm::Thread::joinThread() {
    _thread.join();
}

void gm::Thread::setDebugName(string name) {
    _name = std::move(name);
    _applyDebugName();
}

auto gm::Thread::spawn(delegate<int()> threadMain, string name) -> Thread {
    // the main thread must spawn the first thread, so assign the main thread id here
    if (currentThreadId == ThreadId::None) {
        currentThreadId = ThreadId::Main;
    }

    Thread thread;
    thread._handle = ThreadId(++nextThreadId);
    thread._thread = std::thread([main = std::move(threadMain), id = thread._handle]() mutable {
        currentThreadId = id;
        return main();
    });
    thread._name = std::move(name);
    thread._applyDebugName();
    return thread;
}

void gm::Thread::yieldTimeSlice() {
    std::this_thread::yield();
}
