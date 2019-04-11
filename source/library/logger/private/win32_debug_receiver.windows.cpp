// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/logger/win32_debug_receiver.h"
#include "potato/foundation/platform_windows.h"

void up::Win32DebugReceiver::log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location) noexcept {
    fixed_string_writer<2048> buffer;

    if (location.file) {
        format_into(buffer, "{}({}): <{}> ", location.file, location.line, location.function);
    }

    format_into(buffer, "[{}] {} :: {}\n", toString(severity), loggerName, message);

    OutputDebugStringA(buffer.c_str());
}
