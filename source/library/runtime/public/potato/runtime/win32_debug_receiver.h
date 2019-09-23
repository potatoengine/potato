// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/platform.h"

#if !defined(UP_PLATFORM_WINDOWS)
#    error "win32_debug_receiver.h can only be used on Windows platforms"
#endif

#include "potato/logger/logger.h"

namespace up {
    class Win32DebugReceiver : public LogReceiver {
    public:
        virtual void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept override;
    };
} // namespace up
