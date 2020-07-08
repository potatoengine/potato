// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "win32_debug_receiver.h"

#include "potato/spud/platform_windows.h"

void up::Win32DebugReceiver::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    fixed_string_writer<2048> buffer;

    if (location.file) {
        format_append(buffer, "{}({}): <{}> ", location.file, location.line, location.function);
    }

    format_append(buffer, "[{}] {} :: {}\n", toString(severity), loggerName, message);

    OutputDebugStringA(buffer.c_str());
}
