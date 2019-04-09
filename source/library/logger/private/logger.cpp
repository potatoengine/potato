// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/logger/logger.h"
#include "potato/logger/standard_stream_receiver.h"
#if UP_PLATFORM_WINDOWS
#   include "potato/logger/win32_debug_receiver.h"
#endif

up::Logger::Logger(string name, LogSeverity minimumSeverity) noexcept : _name(std::move(name)), _minimumSeverity(minimumSeverity) {

    static auto standard = up::new_shared<up::StandardStreamReceiver>();
    attach(standard);

    #if UP_PLATFORM_WINDOWS
    static auto debug = up::new_shared<up::Win32DebugReceiver>();
    attach(debug);
    #endif
}
