// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/logger/common.h"
#include "potato/logger/log_receiver.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/string_format.h"
#include "potato/foundation/fixed_string_writer.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/rc.h"
#include "potato/concurrency/rwlock.h"
#include "potato/concurrency/lock_guard.h"
#include <utility>

namespace up {
    class Logger {
    public:
        UP_LOGGER_API Logger(string name, LogSeverity minimumSeverity = LogSeverity::Info) noexcept;
        UP_LOGGER_API Logger(string name, rc<LogReceiver> receiver, LogSeverity minimumSeverity = LogSeverity::Info) noexcept;

        constexpr bool isEnabledFor(LogSeverity severity) const noexcept {
            return severity >= _minimumSeverity;
        }

        template <typename... T>
        void info(string_view format, T const&... args) { _formatDispatch(LogSeverity::Info, format, args...); }
        void info(string_view message) noexcept { _dispatch(LogSeverity::Info, message, {}); }

        template <typename... T>
        void error(string_view format, T const&... args) { _formatDispatch(LogSeverity::Error, format, args...); }
        void error(string_view message) noexcept { _dispatch(LogSeverity::Error, message, {}); }

        void UP_LOGGER_API attach(rc<LogReceiver> receiver) noexcept;
        void UP_LOGGER_API detach(LogReceiver* remove) noexcept;

    protected:
        template <typename... T>
        void _formatDispatch(LogSeverity severity, string_view format, T const&... args) {
            if (!isEnabledFor(severity)) {
                return;
            }

            fixed_string_writer<1024> writer;
            format_into(writer, format, args...);

            _dispatch(severity, writer, {});
        }

        void UP_LOGGER_API _dispatch(LogSeverity severity, string_view message, LogLocation location) noexcept;

    private:
        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
        RWLock _receiversLock;
        vector<rc<LogReceiver>> _receivers;
    };
} // namespace up
