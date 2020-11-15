// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "lock_guard.h"
#include "standard_stream_sink.h"

#include "potato/spud/std_iostream.h"

#include <iostream>

void up::StandardStreamSink::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    std::ostream& os = severity == LogSeverity::Info ? std::cout : std::cerr;

    // Ensure that output isn't interlaced
    //
    LockGuard _(_lock);

    if (location.file) {
        os << location.file << '(' << location.line << "): <" << location.function << "> ";
    }

    os << '[' << toString(severity) << "] " << loggerName << " :: " << message << '\n';

    if (severity != LogSeverity::Info) {
        os.flush();
    }
}
