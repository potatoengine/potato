// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

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
            return LogSeverityMask{ static_cast<unsigned>(a) & static_cast<unsigned>(b) };
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
} // namespace up
