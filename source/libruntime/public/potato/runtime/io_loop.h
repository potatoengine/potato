// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/delegate.h"
#include "potato/spud/span.h"
#include "potato/spud/zstring_view.h"

struct uv_loop_s;
struct uv_pipe_s;

using uv_loop_t = uv_loop_s;
using uv_pipe_t = uv_pipe_s;

namespace up {
    class IOEvent {
    public:
        using Callback = delegate<void()>;

        IOEvent() = default;
        IOEvent(uv_loop_t* loop, Callback callback);
        ~IOEvent() { reset(); }

        IOEvent(IOEvent&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOEvent& operator=(IOEvent&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        UP_RUNTIME_API void signal();

        bool empty() const noexcept { return _state == nullptr; }
        UP_RUNTIME_API void reset();

    private:
        struct State;

        State* _state = nullptr;
    };

    class IOPipe {
    public:
        using ReadCallback = delegate<void(span<char> bytes)>;
        using DisconnectCallback = delegate<void()>;

        IOPipe() = default;
        explicit IOPipe(uv_loop_t* loop);
        IOPipe(uv_loop_t* loop, int fd);
        ~IOPipe() { reset(); }

        IOPipe(IOPipe&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOPipe& operator=(IOPipe&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        UP_RUNTIME_API void startRead(ReadCallback callback);
        UP_RUNTIME_API void stopRead();

        UP_RUNTIME_API void onDisconnect(DisconnectCallback callback);

        UP_RUNTIME_API void write(view<char> bytes);

        UP_RUNTIME_API uv_pipe_t* raw() const noexcept;

        bool empty() const noexcept { return _state == nullptr; }
        UP_RUNTIME_API void reset();

    private:
        struct State;
        struct WriteReq;

        State* _state = nullptr;
    };

    enum class IOWatchEvent { Rename, Change };

    class IOWatch {
    public:
        using Callback = delegate<void(zstring_view filename, IOWatchEvent event)>;

        IOWatch() = default;
        IOWatch(uv_loop_t* loop, zstring_view targetFilename, Callback callback);
        ~IOWatch() { reset(); }

        IOWatch(IOWatch&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOWatch& operator=(IOWatch&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        bool empty() const noexcept { return _state == nullptr; }
        UP_RUNTIME_API void reset();

    private:
        struct State;

        State* _state = nullptr;
    };

    class IOPrepareHook {
    public:
        using Callback = delegate<void()>;

        IOPrepareHook() = default;
        IOPrepareHook(uv_loop_t* loop, Callback callback);
        ~IOPrepareHook() { reset(); }

        IOPrepareHook(IOPrepareHook&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOPrepareHook& operator=(IOPrepareHook&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        bool empty() const noexcept { return _state == nullptr; }
        UP_RUNTIME_API void reset();

    private:
        struct State;

        State* _state = nullptr;
    };

    struct IOProcessConfig {
        zstring_view process;
        view<char const*> args;
        IOPipe* input = nullptr;
        IOPipe* output = nullptr;
    };

    class IOProcess {
    public:
        IOProcess() = default;
        IOProcess(uv_loop_t* loop);
        ~IOProcess() { reset(); }

        IOProcess(IOProcess&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOProcess& operator=(IOProcess&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        UP_RUNTIME_API int spawn(IOProcessConfig const& config);
        UP_RUNTIME_API void terminate(bool kill = false);

        UP_RUNTIME_API int pid() const noexcept;

        UP_RUNTIME_API bool empty() const noexcept;
        UP_RUNTIME_API void reset();

    private:
        struct State;

        State* _state = nullptr;
    };

    enum class IORun { Poll, WaitOne, Default };

    class IOLoop {
    public:
        using PrepareCallback = delegate<void()>;

        UP_RUNTIME_API IOLoop();
        ~IOLoop() { reset(); }

        IOLoop(IOLoop&& rhs) noexcept : _state(rhs._state) { rhs._state = nullptr; }
        IOLoop& operator=(IOLoop&& rhs) noexcept {
            swap(_state, rhs._state);
            return *this;
        }

        explicit operator bool() const noexcept { return !empty(); }

        UP_RUNTIME_API IOEvent createEvent(delegate<void()> callback = {});
        UP_RUNTIME_API IOPipe createPipe();
        UP_RUNTIME_API IOPipe createPipeFor(int fd);
        UP_RUNTIME_API IOWatch createWatch(zstring_view targetFilename, IOWatch::Callback callback);
        UP_RUNTIME_API IOPrepareHook createPrepareHook(IOPrepareHook::Callback callback);
        UP_RUNTIME_API IOProcess createProcess();

        UP_RUNTIME_API bool run(IORun run);

        UP_RUNTIME_API void stop() noexcept;

        static UP_RUNTIME_API zstring_view errorString(int code) noexcept;

        bool empty() const noexcept { return _state == nullptr; }
        UP_RUNTIME_API void reset();

        UP_RUNTIME_API uv_loop_t* raw() const noexcept;

    private:
        struct State;

        State* _state = nullptr;
    };
} // namespace up
