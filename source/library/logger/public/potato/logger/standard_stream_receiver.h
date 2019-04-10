// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/logger/logger.h"
#include "potato/concurrency/spinlock.h"

namespace up {
    class StandardStreamReceiver : public LogReceiver {
    public:
        virtual void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept override;

    private:
        concurrency::Spinlock _lock;
    };
} // namespace up
