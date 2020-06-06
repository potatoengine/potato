// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "lock_guard.h"
#include "log_receiver.h"
#include "log_severity.h"
#include "rwlock.h"

#include "potato/spud/fixed_string_writer.h"
#include "potato/spud/rc.h"
#include "potato/spud/string.h"
#include "potato/spud/string_format.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

#include <utility>

namespace up {
    class Logger {
    public:
        UP_RUNTIME_API Logger(string name, LogSeverity minimumSeverity = LogSeverity::Info);
        UP_RUNTIME_API Logger(string name, rc<LogReceiver> receiver, LogSeverity minimumSeverity = LogSeverity::Info) noexcept;

        constexpr bool isEnabledFor(LogSeverity severity) const noexcept { return severity >= _minimumSeverity; }

        template <typename... T> void info(string_view format, T const&... args) { _formatDispatch(LogSeverity::Info, format, args...); }
        void info(string_view message) noexcept { _dispatch(LogSeverity::Info, message, {}); }

        template <typename... T> void error(string_view format, T const&... args) { _formatDispatch(LogSeverity::Error, format, args...); }
        void error(string_view message) noexcept { _dispatch(LogSeverity::Error, message, {}); }

        void UP_RUNTIME_API attach(rc<LogReceiver> receiver) noexcept;
        void UP_RUNTIME_API detach(LogReceiver* remove) noexcept;

    protected:
        template <typename... T> void _formatDispatch(LogSeverity severity, string_view format, T const&... args);

        void UP_RUNTIME_API _dispatch(LogSeverity severity, string_view message, LogLocation location) noexcept;

    private:
        static constexpr int log_length = 1024;

        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
        RWLock _receiversLock;
        vector<rc<LogReceiver>> _receivers;
    };

    template <typename... T> void Logger::_formatDispatch(LogSeverity severity, string_view format, T const&... args) {
        if (!isEnabledFor(severity)) {
            return;
        }

        fixed_string_writer<log_length> writer;
        format_append(writer, format, args...);

        _dispatch(severity, writer, {});
    }
} // namespace up
