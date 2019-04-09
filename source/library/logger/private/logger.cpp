// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/logger/logger.h"
#include "potato/logger/standard_stream_receiver.h"
#include <iostream>

static up::rc<up::LogReceiver> defaultReceiver() noexcept {
    static auto receiver = up::new_shared<up::StandardStreamReceiver>();
    return receiver;
}

up::Logger::Logger(string name, LogSeverity minimumSeverity) noexcept
    : _name(std::move(name))
    , _minimumSeverity(minimumSeverity) {
    attach(defaultReceiver());
}
