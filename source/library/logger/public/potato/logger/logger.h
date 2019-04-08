// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/string_format.h"
#include "potato/foundation/fixed_string_writer.h"
#include "potato/foundation/vector.h"
#include "potato/foundation/rc.h"
#include "potato/concurrency/spinlock.h"
#include "potato/concurrency/lock_guard.h"
#include <utility>

namespace up {
    struct LogLocation {
        zstring_view file;
        zstring_view function;
        int line = 0;
    };

    enum class LogSeverity {
        Info,
        Error
    };

    class LogReceiver : public shared<LogReceiver> {
    public:
        virtual ~LogReceiver() = default;

        virtual void log(LogSeverity severity, string_view message, LogLocation location = {}) noexcept = 0;
    };

    class Logger {
    public:
        UP_LOGGER_API Logger(string name, LogSeverity minimumSeverity = LogSeverity::Info) noexcept;

        Logger(string name, rc<LogReceiver> receiver, LogSeverity minimumSeverity = LogSeverity::Info) noexcept : _name(std::move(name)), _minimumSeverity(minimumSeverity) {
            attach(std::move(receiver));
        }

        constexpr bool isEnabledFor(LogSeverity severity) const noexcept {
            return severity >= _minimumSeverity;
        }

        template <typename... T>
        void info(string_view format, T const&... args) { _formatDispatch(LogSeverity::Info, format, args...); }
        void info(string_view message) noexcept { _dispatch(LogSeverity::Info, message, {}); }

        template <typename... T>
        void error(string_view format, T const&... args) { _formatDispatch(LogSeverity::Error, format, args...); }
        void error(string_view message) noexcept { _dispatch(LogSeverity::Error, message, {}); }

        void attach(rc<LogReceiver> receiver) noexcept {
            concurrency::LockGuard _(_receiversLock);
            _receivers.push_back(std::move(receiver));
        }

        void detach(LogReceiver* remove) noexcept {
            concurrency::LockGuard _(_receiversLock);
            for (size_t i = 0; i != _receivers.size(); ++i) {
                if (_receivers[i].get() == remove) {
                    _receivers.erase(_receivers.begin() + i);
                    --i;
                }
            }
        }

    protected:
        template <typename... T>
        void _formatDispatch(LogSeverity severity, string_view format, T const&... args) {
            fixed_string_writer<1024> writer;
            format_into(writer, format, args...);
            _dispatch(severity, writer, {});
        }

        void _dispatch(LogSeverity severity, string_view message, LogLocation location) noexcept {
            concurrency::LockGuard _(_receiversLock);
            for (auto& receiver : _receivers) {
                receiver->log(severity, message, location);
            }
        }

    private:
        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
        concurrency::Spinlock _receiversLock;
        vector<rc<LogReceiver>> _receivers;
    };

    class DefaultLogReceiver final : public LogReceiver {
    public:
        void UP_LOGGER_API log(LogSeverity severity, string_view message, LogLocation location = {}) noexcept override;
    };
} // namespace up
