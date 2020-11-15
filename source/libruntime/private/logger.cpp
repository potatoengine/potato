// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "logger.h"
#include "standard_stream_sink.h"
#if UP_PLATFORM_WINDOWS
#    include "win32_debug_sink.h"
#endif

up::Logger::Logger(string name, LogSeverity minimumSeverity)
    : _name(std::move(name))
    , _minimumSeverity(minimumSeverity) {
    static auto standard = up::new_shared<up::StandardStreamSink>();
    attach(standard);

#if UP_PLATFORM_WINDOWS
    static auto debug = up::new_shared<up::Win32DebugSink>();
    attach(debug);
#endif
}

up::Logger::Logger(string name, rc<LogSink> receiver, LogSeverity minimumSeverity) noexcept
    : _name(std::move(name))
    , _minimumSeverity(minimumSeverity) {
    attach(std::move(receiver));
}

void up::Logger::attach(rc<LogSink> receiver) noexcept {
    LockGuard _(_receiversLock.writer());
    _receivers.push_back(std::move(receiver));
}

void up::Logger::detach(LogSink* remove) noexcept {
    LockGuard _(_receiversLock.writer());
    for (size_t i = 0; i != _receivers.size(); ++i) {
        if (_receivers[i].get() == remove) {
            _receivers.erase(_receivers.begin() + i);
            --i;
        }
    }
}

void up::Logger::log(LogSeverity severity, string_view message) noexcept {
    if (!isEnabledFor(severity)) {
        return;
    }

    LockGuard _(_receiversLock.reader());
    for (auto& receiver : _receivers) {
        receiver->log(_name, severity, message, {});
    }
}
