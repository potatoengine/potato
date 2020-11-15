// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "logger.h"

#if UP_PLATFORM_WINDOWS
#    include "potato/spud/platform_windows.h"
#endif

#include <iostream>

void up::DefaultLogSink::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    char buffer[2048] = {
        0,
    };

    if (location.file) {
        format_append(buffer, "{}({}): <{}> ", location.file, location.line, location.function);
    }

    format_append(buffer, "[{}] {} :: {}\n", toString(severity), loggerName, message);

    {
        std::ostream& os = severity == LogSeverity::Error ? std::cerr : std::cout;

        os << buffer;

        if (severity != LogSeverity::Info) {
            os.flush();
        }
    }

#if defined(UP_PLATFORM_WINDOWS)
    OutputDebugStringA(buffer);
#endif
}

up::Logger::Logger(string name, rc<Impl> parent, rc<LogSink> receiver, LogSeverity minimumSeverity)
    : _impl(new_shared<Impl>()) {
    _impl->name = std::move(name);
    _impl->parent = std::move(parent);
    _impl->minimumSeverity = minimumSeverity;
    if (receiver != nullptr) {
        _impl->receivers.push_back(std::move(receiver));
    }
}

up::Logger::~Logger() = default;

auto up::Logger::root() -> up::Logger& {
    static Logger s_logger("Potato", nullptr, new_shared<DefaultLogSink>(), LogSeverity::Info);
    return s_logger;
}

bool up::Logger::isEnabledFor(LogSeverity severity) const noexcept {
    return severity >= _impl->minimumSeverity;
}

void up::Logger::attach(rc<LogSink> receiver) noexcept {
    if (receiver.empty()) {
        return;
    }

    LockGuard _(_impl->lock.writer());
    _impl->receivers.push_back(std::move(receiver));
}

void up::Logger::detach(LogSink* remove) noexcept {
    if (remove == nullptr) {
        return;
    }

    LockGuard _(_impl->lock.writer());
    auto& receivers = _impl->receivers;
    for (size_t i = 0; i != receivers.size(); ++i) {
        if (receivers[i].get() == remove) {
            receivers.erase(receivers.begin() + i);
            --i;
        }
    }
}

void up::Logger::_dispatch(Impl& impl, LogSeverity severity, string_view loggerName, string_view message) noexcept {
    rc<Impl> parent;

    {
        LockGuard _(impl.lock.reader());
        for (auto& receiver : impl.receivers) {
            receiver->log(loggerName, severity, message, {});
        }
        parent = impl.parent;
    }

    if (parent != nullptr && severity >= parent->minimumSeverity) {
        _dispatch(*parent, severity, loggerName, message);
    }
}

void up::Logger::log(LogSeverity severity, string_view message) noexcept {
    if (!isEnabledFor(severity)) {
        return;
    }

    rc<Impl> impl = _impl;

    _dispatch(*impl, severity, impl->name, message);
}
