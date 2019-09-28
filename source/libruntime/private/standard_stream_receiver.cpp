// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/runtime/standard_stream_receiver.h"
#include "potato/runtime/lock_guard.h"
#include "potato/spud/std_iostream.h"
#include <iostream>

void up::StandardStreamReceiver::log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location) noexcept {
    std::ostream& os = severity == LogSeverity::Info ? std::cout : std::cerr;

    // Ensure that output isn't interlaced
    //
    LockGuard _(_lock);

    if (location.file) {
        os << location.file << '(' << location.line << "): <" << location.function << "> ";
    }

    os << '[' << toString(severity) << "] " << loggerName << " :: " << message << '\n';

    if (severity != LogSeverity::Info) {
        os << std::flush;
    }
}