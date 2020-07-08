// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/spud/platform.h"

#if !defined(UP_PLATFORM_WINDOWS)
#    error "win32_debug_receiver.h can only be used on Windows platforms"
#endif

#include "logger.h"

namespace up {
    class Win32DebugReceiver final : public LogReceiver {
    public:
        void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept
            override;
    };
} // namespace up
