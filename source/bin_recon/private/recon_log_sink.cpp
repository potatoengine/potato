// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_log_sink.h"

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>
#include <iostream>

void up::recon::ReconProtocolLogSink::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    schema::ReconLogMessage msg;
    msg.category = string(loggerName);
    msg.message = string(message);
    msg.severity = severity;

    nlohmann::json doc;
    if (reflex::encodeToJson(doc, msg)) {
        std::cout << doc.dump() << std::endl;
    }
}
