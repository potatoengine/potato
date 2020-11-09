// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/log_receiver.h"

namespace up::recon {
    class ReconProtocolLogReceiver : public LogReceiver {
    public:
        ~ReconProtocolLogReceiver() override = default;

        LogResult log(
            string_view loggerName,
            LogSeverity severity,
            string_view message,
            LogLocation location = {}) noexcept override;
    };
} // namespace up::recon
