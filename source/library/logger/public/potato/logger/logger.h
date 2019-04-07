// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"
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

        virtual void logMessage(LogSeverity severity, string_view message, LogLocation location = {}) noexcept = 0;

    protected:
        Logger(string name, LogSeverity minimumSeverity) noexcept : _name(std::move(name)), _minimumSeverity(minimumSeverity) {}

    private:
        string _name;
        LogSeverity _minimumSeverity = LogSeverity::Info;
    };

    class DefaultLogger final : public Logger {
    public:
        DefaultLogger(string name) noexcept : Logger(std::move(name), LogSeverity::Info) {}

        void UP_LOGGER_API logMessage(LogSeverity severity, string_view message, LogLocation location = {}) noexcept override;
    };
} // namespace up
