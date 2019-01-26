// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include <grimm/foundation/types.h>
#include <grimm/foundation/box.h>
#include <grimm/foundation/delegate.h>
#include <grimm/foundation/string_view.h>
#include <grimm/foundation/string_blob.h>

namespace gm {

    enum class ThreadId : uint64 { None = 0 };

    /// <summary> Thread wrapper. </summary>
    class Thread {
    public:
        Thread() = default;
        ~Thread() { joinThread(); }

        Thread(Thread const&) = delete;
        Thread& operator=(Thread const&) = delete;

        Thread(Thread&& rhs) : _handle(rhs._handle), _threadMain(std::move(rhs._threadMain)), _name(std::move(rhs._name)) { rhs._handle = ThreadId::None; }
        Thread& operator=(Thread&& rhs) {
            if (this != &rhs) {
                joinThread();

                _handle = rhs._handle;
                _threadMain = std::move(rhs._threadMain);
                _name = std::move(rhs._name);

                rhs._handle = ThreadId::None;
            }
            return *this;
        }

        ThreadId getThreadId() const { return _handle; }
        GM_CONCURRENCY_API static ThreadId getCurrentThreadId();

        GM_CONCURRENCY_API void joinThread();

        GM_CONCURRENCY_API void setDebugName(string name);

        string const& getDebugName() const { return _name; }

        GM_CONCURRENCY_API static Thread spawn(delegate<int()> threadMain, string name = {});
        GM_CONCURRENCY_API static void yieldTimeSlice();

    private:
        ThreadId _handle = ThreadId::None;
        box<delegate<int()>> _threadMain;
        string _name;
    };

} // namespace gm
