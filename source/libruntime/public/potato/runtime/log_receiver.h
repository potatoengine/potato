// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "log_severity.h"
#include "potato/spud/string_view.h"
#include "potato/spud/rc.h"
#include <utility>

namespace up {
    struct LogLocation {
        zstring_view file;
        zstring_view function;
        int line = 0;
    };

    class LogReceiver : public shared<LogReceiver> {
    public:
        virtual ~LogReceiver() = default;

        virtual void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept = 0;
    };
} // namespace up
