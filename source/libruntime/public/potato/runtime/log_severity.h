// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

namespace up {
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
        return static_cast<LogSeverityMask>(1 << static_cast<int>(severity));
    }

    constexpr LogSeverityMask toInclusiveMask(LogSeverity severity) noexcept {
        unsigned const high = 1 << static_cast<int>(severity);
        unsigned const rest = high - 1;
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
