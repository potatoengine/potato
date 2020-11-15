// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "lock_guard.h"
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
    enum class LogSeverity { Info, Error };

    namespace log_severity_mask {
        enum class LogSeverityMask : unsigned {
            Info = 1 << (int)LogSeverity::Info,
            Error = 1 << (int)LogSeverity::Error,
            Everything = Info | Error
        };

        constexpr LogSeverityMask operator~(LogSeverityMask mask) noexcept {
            return LogSeverityMask{~static_cast<unsigned>(mask)};
        }

        constexpr LogSeverityMask operator|(LogSeverityMask a, LogSeverityMask b) noexcept {
            return LogSeverityMask{static_cast<unsigned>(a) | static_cast<unsigned>(b)};
        }

        constexpr LogSeverityMask operator&(LogSeverityMask a, LogSeverityMask b) noexcept {
            return LogSeverityMask{static_cast<unsigned>(a) & static_cast<unsigned>(b)};
        }
    } // namespace log_severity_mask

    using LogSeverityMask = log_severity_mask::LogSeverityMask;

    constexpr LogSeverityMask toMask(LogSeverity severity) noexcept {
        return static_cast<LogSeverityMask>(1 << static_cast<int>(severity));
    }

    constexpr LogSeverityMask toInclusiveMask(LogSeverity severity) noexcept {
        unsigned const high = 1 << static_cast<int>(severity);
        unsigned const rest = high - 1;
        return static_cast<LogSeverityMask>(high | rest);
    }

    constexpr zstring_view toString(LogSeverity severity) noexcept {
        switch (severity) {
            case LogSeverity::Info:
                return "info"_zsv;
            case LogSeverity::Error:
                return "error"_zsv;
            default:
                return "unknown"_zsv;
        }
    }

    struct LogLocation {
        zstring_view file;
        zstring_view function;
        int line = 0;
    };

    class LogReceiver : public shared<LogReceiver> {
    public:
        virtual ~LogReceiver() = default;

        virtual void log(
            string_view loggerName,
            LogSeverity severity,
            string_view message,
            LogLocation location = {}) noexcept = 0;
    };

    class Logger {
    public:
        UP_RUNTIME_API Logger(string name, LogSeverity minimumSeverity = LogSeverity::Info);
        UP_RUNTIME_API
        Logger(string name, rc<LogReceiver> receiver, LogSeverity minimumSeverity = LogSeverity::Info) noexcept;

        constexpr bool isEnabledFor(LogSeverity severity) const noexcept { return severity >= _minimumSeverity; }

        template <typename... T>
        void log(LogSeverity severity, string_view format, T const&... args);
        void UP_RUNTIME_API log(LogSeverity severity, string_view message) noexcept;

        template <typename... T>
        void info(string_view format, T const&... args) {
            log(LogSeverity::Info, format, args...);
        }
        void info(string_view message) noexcept { log(LogSeverity::Info, message); }

        template <typename... T>
        void error(string_view format, T const&... args) {
            log(LogSeverity::Error, format, args...);
        }
        void error(string_view message) noexcept { log(LogSeverity::Error, message); }

        void UP_RUNTIME_API attach(rc<LogReceiver> receiver) noexcept;
        void UP_RUNTIME_API detach(LogReceiver* remove) noexcept;

    private:
        static constexpr int log_length = 1024;

        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
        RWLock _receiversLock;
        vector<rc<LogReceiver>> _receivers;
    };

    template <typename... T>
    void Logger::log(LogSeverity severity, string_view format, T const&... args) {
        if (!isEnabledFor(severity)) {
            return;
        }

        fixed_string_writer<log_length> writer;
        format_append(writer, format, args...);

        log(severity, writer);
    }
} // namespace up
