// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "logger.h"
#include "spinlock.h"

namespace up {
    class StandardStreamReceiver final : public LogReceiver {
    public:
        void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept
            override;

    private:
        Spinlock _lock;
    };
} // namespace up
