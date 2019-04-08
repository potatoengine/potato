// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/logger/logger.h"
#include "potato/foundation/std_iostream.h"
#include <iostream>

static up::rc<up::LogReceiver> defaultReceiver() noexcept {
    static auto receiver = up::new_shared<up::DefaultLogReceiver>();
    return receiver;
}

up::Logger::Logger(string name, LogSeverity minimumSeverity) noexcept
    : _name(std::move(name))
    , _minimumSeverity(minimumSeverity) {
    attach(defaultReceiver());
}

void up::DefaultLogReceiver::log(LogSeverity severity, string_view message, LogLocation location) noexcept {
    std::ostream& os = severity == LogSeverity::Info ? std::cout : std::cerr;

    if (location.file) {
        os << location.file << '(' << location.line << ") [" << location.function << "] ";
    }

    os << message << '\n';
}
