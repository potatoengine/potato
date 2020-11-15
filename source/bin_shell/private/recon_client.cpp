// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/tools/project.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>
#include <reproc++/drain.hpp>

static up::Logger s_logger("ReconClient");

namespace {
    struct ReconClientStreamSink {
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

        bool handleLine(up::string_view line) {
            nlohmann::json doc = nlohmann::json::parse(line, nullptr, false, true);
            if (!doc.is_object()) {
                return false;
            }

            auto const& type = doc["$type"].get<up::string_view>();
            if (type == "log") {
                auto const severityLabel = doc["severity"].get<up::string_view>();
                auto const message = doc["message"].get<up::string_view>();

                up::LogSeverity severity =
                    severityLabel == toString(up::LogSeverity::Error) ? up::LogSeverity::Error : up::LogSeverity::Info;

                s_logger.log(severity, message);
            }

            return true;
        }
    };
} // namespace

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
        ReconClientStreamSink sink;
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
