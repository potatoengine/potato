// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include <grimm/foundation/rc.h>
#include <grimm/foundation/delegate.h>
#include <atomic>

namespace gm {
    class Task : public shared<Task> {
    public:
        inline void schedule();
        inline void execute();
        bool isReady() const { return 0 == _pending.load(std::memory_order_relaxed); }

        Task(delegate<void()> work) : _work(std::move(work)) {}

        Task(Task&&) = delete;
        Task& operator=(Task&&) = delete;

    private:
        std::atomic<int> _pending = 0;
        delegate<void()> _work;
    };

    void Task::schedule() {
        _pending.fetch_add(1, std::memory_order_acquire);
    }

    void Task::execute() {
        _work();
        _work = nullptr;
        _pending.fetch_sub(1, std::memory_order_release);
    }

} // namespace gm
