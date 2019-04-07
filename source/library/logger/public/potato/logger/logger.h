// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"
#include "potato/foundation/string_format.h"
#include "potato/foundation/fixed_string_writer.h"
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

    class Logger {
    public:
        virtual ~Logger() = default;

        constexpr bool isEnabledFor(LogSeverity severity) const noexcept {
            return severity >= _minimumSeverity;
        }

        template <typename... T>
        void info(string_view format, T const&... args) { _log(LogSeverity::Info, format, args...); }
        void info(string_view message) noexcept { log(LogSeverity::Info, message); }

        template <typename... T>
        void error(string_view format, T const&... args) { _log(LogSeverity::Error, format, args...); }
        void error(string_view message) noexcept { log(LogSeverity::Error, message); }

        virtual void log(LogSeverity severity, string_view message, LogLocation location = {}) noexcept = 0;

    protected:
        Logger(string name, LogSeverity minimumSeverity) noexcept : _name(std::move(name)), _minimumSeverity(minimumSeverity) {}

        template <typename... T>
        void _log(LogSeverity severity, string_view format, T const&... args) {
            fixed_string_writer<1024> writer;
            format_into(writer, format, args...);
            log(severity, writer);
        }

    private:
        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
    };

    class DefaultLogger final : public Logger {
    public:
        DefaultLogger(string name) noexcept : Logger(std::move(name), LogSeverity::Info) {}

        void UP_LOGGER_API log(LogSeverity severity, string_view message, LogLocation location = {}) noexcept override;
    };
} // namespace up
