// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/tools/project.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>
#include <reproc++/drain.hpp>

static up::Logger s_logger("ReconClient");

struct up::shell::ReconClient::ReprocSink {
    ReconClient& client;

    auto operator()(reproc::stream stream, uint8_t const* buffer, size_t bytes) -> std::error_code {
        uint8_t const* start = buffer;
        uint8_t const* const end = buffer + bytes;

        // FIXME: this assumes we're getting whole lines, which is not guaranteed!
        for (uint8_t const* c = buffer; c != end; ++c) {
            if (*c == '\n') {
                handleLine({reinterpret_cast<char const*>(start), static_cast<size_t>(c - start)});
                start = c + 1;
            }
        }

        return {};
    }

    bool handleLine(string_view line) {
        nlohmann::json doc = nlohmann::json::parse(line, nullptr, false, true);
        if (!doc.is_object()) {
            return false;
        }

        auto const& type = doc["$type"].get<string_view>();
        if (type == "log") {
            auto const severityLabel = doc["severity"].get<string_view>();
            auto const message = doc["message"].get<string_view>();

            LogSeverity severity =
                severityLabel == toString(LogSeverity::Error) ? LogSeverity::Error : LogSeverity::Info;

            s_logger.log(severity, message);

            client._staleAssets.store(true);
        }

        return true;
    }
};

bool up::shell::ReconClient::start(Project& project) {
    stop();

    reproc::options options = {

    };

    const char* const args[] = {"recon", "-project", project.projectFilePath().c_str(), "-server", nullptr};
    auto const ec = _process.start(args);
    if (ec) {
        s_logger.error("Failed to start recon: {}", ec.message());
        return false;
    }

    _thread = std::thread([this] {
        ReprocSink sink{.client = *this};
        auto const ec = reproc::drain(_process, sink, sink);
        if (ec) {
            s_logger.error("Processing recon output failed: {}", ec.message());
        }
    });

    auto [pid, _] = _process.pid();
    s_logger.info("Started recon PID={}", pid);

    return true;
}

void up::shell::ReconClient::stop() {
    _process.kill();

    if (_thread.joinable()) {
        _thread.join();
    }
}

bool up::shell::ReconClient::hasUpdatedAssets() noexcept {
    return _staleAssets.exchange(false);
}
