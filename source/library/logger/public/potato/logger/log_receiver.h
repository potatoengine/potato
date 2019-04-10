// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "_export.h"
#include "potato/logger/common.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/rc.h"
#include <utility>

namespace up {
    class LogReceiver : public shared<LogReceiver> {
    public:
        virtual ~LogReceiver() = default;

        virtual void log(LogSeverity severity, string_view message, LogLocation location = {}) noexcept = 0;
    };
} // namespace up
