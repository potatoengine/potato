// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/recon/recon_protocol.h"
#include "potato/tools/project.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>
#include <reproc++/drain.hpp>

static up::Logger s_logger("ReconClient"); // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

struct up::ReconClient::ReprocSink {
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

        auto handleLog = [this](schema::ReconLogMessage const& msg) {
            client._handle(msg);
        };
        auto handleManifest = [this](schema::ReconManifestMessage const& msg) {
            client._handle(msg);
        };

        return decodeReconMessage<schema::ReconLogMessage>(doc, handleLog) ||
            decodeReconMessage<schema::ReconManifestMessage>(doc, handleManifest);
    }
};

bool up::ReconClient::start(Project& project) {
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

void up::ReconClient::stop() {
    _process.kill();

    if (_thread.joinable()) {
        _thread.join();
    }
}

bool up::ReconClient::hasUpdatedAssets() noexcept {
    return _staleAssets.exchange(false);
}

void up::ReconClient::_handle(schema::ReconLogMessage const& msg) {
    s_logger.log(msg.severity, msg.message);
}

void up::ReconClient::_handle(schema::ReconManifestMessage const& msg) {
    _staleAssets.store(true);
}

bool up::ReconClient::_sendRaw(reflex::Schema const& schema, void const* object) {
    nlohmann::json doc;
    if (!reflex::encodeToJsonRaw(doc, schema, object)) {
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
