// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/logger/logger.h"
#include "potato/foundation/std_iostream.h"
#include <iostream>

void up::DefaultLogger::log(LogSeverity severity, string_view message, LogLocation location) noexcept {
    std::ostream& os = severity == LogSeverity::Info ? std::cout : std::cerr;

    if (location.file) {
        os << location.file << '(' << location.line << ") [" << location.function << "] ";
    }

    os << message << '\n';
}
