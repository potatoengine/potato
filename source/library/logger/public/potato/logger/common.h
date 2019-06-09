// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"

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

    enum class LogSeverityMask : unsigned {
        Info = 1 << (int)LogSeverity::Info,
        Error = 1 << (int)LogSeverity::Error,
        Everything = Info | Error
    };

    constexpr LogSeverityMask toMask(LogSeverity severity) noexcept {
        return static_cast<LogSeverityMask>(1 << (int)severity);
    }

    constexpr LogSeverityMask toInclusiveMask(LogSeverity severity) noexcept {
        unsigned high = 1 << (int)severity;
        unsigned rest = high - 1;
        return static_cast<LogSeverityMask>(high | rest);
    }

    constexpr string_view toString(LogSeverity severity) noexcept {
        switch (severity) {
        case LogSeverity::Info: return "info";
        case LogSeverity::Error: return "error";
        default: return "unknown";
        }
    }
} // namespace up
