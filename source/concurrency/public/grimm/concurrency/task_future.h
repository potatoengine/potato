// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "Task.h"
#include "TaskQueue.h"
#include <gmx/shared.h>
#include <gmx/optional.h>

namespace gm {

    template <typename ResultT>
    class TaskFuture;
    template <typename WorkT>
    TaskFuture<decltype(work())> spawn(WorkT&& work);

    template <>
    class TaskFuture<void> {
    public:
        TaskFuture(shared_ptr<Task> task) : _task(move(task)) {}

        bool isReady() const { return _task == nullptr || _task->isReady(); }
        inline void await();

        static inline TaskFuture spawn(delegate<void()> work);

    private:
        shared_ptr<Task> _task;
    };

    template <typename ResultT>
    class TaskFuture {
    public:
        bool isReady() const { return _task == nullptr || _task->isReady(); }
        ResultT const& await();

        static TaskFuture<ResultT> spawn(delegate<ResultT()> work);

    private:
        struct Result : shared_base<Result> {
            delegate<ResultT()> work;
            optional<ResultT> value;

            Result(delegate<ResultT()> w) : work(move(w)) {}
        };

        TaskFuture(shared_ptr<Result> result, shared_ptr<Task> item) : _result(move(result)), _task(move(item)) {}

        shared_ptr<Result> _result;
        shared_ptr<Task> _task;
    };

    void TaskFuture<void>::await() {
        if (_task) {
            TaskQueue::getInstance().await(*_task);
            _task.reset();
        }
    }

    TaskFuture<void> TaskFuture<void>::spawn(delegate<void()> work) {
        TaskFuture<void> future(make_shared<Task>(move(work)));
        TaskQueue::getInstance().enque(future._task);
        return future;
    }

    template <typename ResultT>
    TaskFuture<ResultT> TaskFuture<ResultT>::spawn(delegate<ResultT()> work) {
        auto result = make_shared<Result>(move(work));
        auto task = make_shared<Task>([result]() { result->value = result->work(); });
        TaskQueue::getInstance().enque(task);
        return TaskFuture<ResultT>(move(result), move(task));
    }

    template <typename ResultT>
    ResultT const& TaskFuture<ResultT>::await() {
        if (_task) {
            TaskQueue::getInstance().await(*_task);
            _task.reset();
            _result->work.reset(); // release any resources held by the delegate
        }

        return *_result->value;
    }

    template <typename WorkT>
    auto spawn(WorkT&& work) -> TaskFuture<decltype(work())> {
        return TaskFuture<decltype(work())>::spawn(std::forward<WorkT>(work));
    }

} // namespace gm
