// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/logger/log_severity.h"
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
