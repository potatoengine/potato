// Copyright (C) 2016,2019 Sean Middleditch, all rights reserverd.

#include "thread.h"
#include <grimm/foundation/delegate.h>
#include <grimm/foundation/string_view.h>

#if defined(GM_PLATFORM_WINDOWS)
#    include <grimm/foundation/platform_windows.h>
#endif

auto gm::Thread::getCurrentThreadId() -> ThreadId {
#if defined(GM_PLATFORM_WINDOWS)
    return static_cast<ThreadId>(reinterpret_cast<uintptr>(GetCurrentThread()));
#else
#    error "Unsupported platform"
#endif
}

void gm::Thread::joinThread() {
    if (_handle != ThreadId::None) {
#if defined(GM_PLATFORM_WINDOWS)
        WaitForSingleObject(reinterpret_cast<HANDLE>(_handle), INFINITE);
#else
#    error "Unsupported platform"
#endif
    }
}

#if defined(GM_PLATFORM_WINDOWS)
// https://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
static void SetWindowsThreadName(DWORD threadId, char const* name) {
    constexpr DWORD MS_VC_EXCEPTION = 0x406D1388;

#    pragma pack(push, 8)
    typedef struct tagTHREADNAME_INFO {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#    pragma pack(pop)

    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = name;
    info.dwThreadID = threadId;
    info.dwFlags = 0;

#    pragma warning(push)
#    pragma warning(disable : 6320 6322)
    __try {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
    }
#    pragma warning(pop)
}
#endif // defined(GM_PLATFORM_WINDOWS)

void gm::Thread::setDebugName(string name) {
    _name = std::move(name);

#if defined(GM_PLATFORM_WINDOWS)
    SetWindowsThreadName(GetThreadId(reinterpret_cast<HANDLE>(_handle)), name.c_str());
#else
#    error "Unsupported platform"
#endif
}

auto gm::Thread::spawn(delegate<int()> threadMain, string name) -> Thread {
    Thread thread;
    thread._threadMain = make_box<delegate<int()>>(std::move(threadMain));
    thread._name = std::move(name);

#if defined(GM_PLATFORM_WINDOWS)
    DWORD threadId = 0;
    HANDLE handle = CreateThread(nullptr, 0, [](void* main) { return static_cast<DWORD>((*static_cast<delegate<int()>*>(main))()); }, thread._threadMain.get(), CREATE_SUSPENDED, &threadId);
    GM_ASSERT(handle != 0);
    thread._handle = static_cast<ThreadId>(reinterpret_cast<uintptr>(handle));
    SetWindowsThreadName(threadId, thread._name.c_str());
    ResumeThread(handle);
#else
#    error "Unsupported platformed"
#endif

    return thread;
}

void gm::Thread::yieldTimeSlice() {
#if defined(GM_PLATFORM_WINDOWS)
    SwitchToThread();
#else
#    error "Unsupported platformed"
#endif
}
