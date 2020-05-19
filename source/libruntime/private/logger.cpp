// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "potato/runtime/logger.h"
#include "potato/runtime/standard_stream_receiver.h"
#if UP_PLATFORM_WINDOWS
#    include "potato/runtime/win32_debug_receiver.h"
#endif

up::Logger::Logger(string name, LogSeverity minimumSeverity) : _name(std::move(name)), _minimumSeverity(minimumSeverity) {
    static auto standard = up::new_shared<up::StandardStreamReceiver>();
    attach(standard);

#if UP_PLATFORM_WINDOWS
    static auto debug = up::new_shared<up::Win32DebugReceiver>();
    attach(debug);
#endif
}

up::Logger::Logger(string name, rc<LogReceiver> receiver, LogSeverity minimumSeverity) noexcept : _name(std::move(name)), _minimumSeverity(minimumSeverity) {
    attach(std::move(receiver));
}

void up::Logger::attach(rc<LogReceiver> receiver) noexcept {
    LockGuard _(_receiversLock.writer());
    _receivers.push_back(std::move(receiver));
}

void up::Logger::detach(LogReceiver* remove) noexcept {
    LockGuard _(_receiversLock.writer());
    for (size_t i = 0; i != _receivers.size(); ++i) {
        if (_receivers[i].get() == remove) {
            _receivers.erase(_receivers.begin() + i);
            --i;
        }
    }
}

void up::Logger::_dispatch(LogSeverity severity, string_view message, LogLocation location) noexcept {
    if (!isEnabledFor(severity)) {
        return;
    }

    LockGuard _(_receiversLock.reader());
    for (auto& receiver : _receivers) {
        receiver->log(_name, severity, message, location);
    }
}
