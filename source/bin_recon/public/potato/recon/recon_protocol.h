// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/logger.h"

namespace up::recon {
    class ReconProtocolLogSink : public LogSink {
    public:
        ~ReconProtocolLogSink() override = default;

        void log(string_view loggerName, LogSeverity severity, string_view message, LogLocation location = {}) noexcept
            override;
    };
} // namespace up::recon
