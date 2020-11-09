// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_protocol.h"

#include "potato/runtime/json.h"

#include <nlohmann/json.hpp>
#include <iostream>

up::LogResult up::recon::ReconProtocolLogReceiver::log(
    string_view loggerName,
    LogSeverity severity,
    string_view message,
    LogLocation location) noexcept {
    nlohmann::json doc;

    doc = {
        {"$type", "log"},
        {"severity", toString(severity)},
        {"message", message},
        {"location", {{"file", location.file}, {"function", location.function}, {"line", location.line}}}};

    std::cout << doc.dump() << std::endl;

    return LogResult::Break;
}
