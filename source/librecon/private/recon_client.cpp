// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/recon/recon_protocol.h"
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

        box<schema::ReconMessage> msg;
        reflex::Schema const* schema = nullptr;

        if (!recon::decodeReconMessage(doc, msg, schema)) {
            return false;
        }

        return client.handleMessage(*schema, *msg);
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

bool up::shell::ReconClient::handleMessage(reflex::Schema const& schema, schema::ReconMessage const& msg) {
    static reflex::Schema const& logSchema = reflex::getSchema<schema::ReconLogMessage>();
    static reflex::Schema const& manifestSchema = reflex::getSchema<schema::ReconManifestMessage>();

    if (schema.name == logSchema.name) {
        auto const& log = static_cast<schema::ReconLogMessage const&>(msg);
        s_logger.log(log.severity, log.message);
        return true;
    }

    if (schema.name == manifestSchema.name) {
        _staleAssets.store(true);
        return true;
    }

    return false;
}

bool up::shell::ReconClient::sendMessage(reflex::Schema const& schema, schema::ReconMessage const& msg) {
    nlohmann::json doc;
    if (!recon::encodeReconMessageRaw(doc, schema, &msg)) {
        return false;
    }

    auto const str = doc.dump() + "\r\n";
    auto const [bytes, ec] = _process.write(reinterpret_cast<uint8 const*>(str.data()), str.size());
    if (ec) {
        s_logger.log(LogSeverity::Error, "Failed to write `{}` message to recon: {}", schema.name, ec.message());
        return false;
    }
    if (bytes != str.size()) {
        s_logger.log(
            LogSeverity::Error,
            "Failed to write full `{}` message to recon: {} byte written of {}",
            schema.name,
            bytes,
            str.size());
        return false;
    }

    return true;
}
