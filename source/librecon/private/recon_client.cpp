// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>

static up::Logger s_logger("ReconClient"); // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void up::ReconClient::_handleLine(string_view line) {
    using namespace up::schema;

    if (line.empty()) {
        return;
    }

    nlohmann::json doc = nlohmann::json::parse(line, nullptr, false, true);

    auto handleLog = [this](ReconLogMessage const& msg) {
        _handle(msg);
    };
    auto handleManifest = [this](ReconManifestMessage const& msg) {
        _handle(msg);
    };

    decodeReconMessage<ReconLogMessage>(doc, handleLog) ||
        decodeReconMessage<ReconManifestMessage>(doc, handleManifest);
}

void up::ReconClient::_onRead(span<char> input) {
    // FIXME: this assumes we're getting whole lines, which is not guaranteed!
    char const* start = input.data();
    char const* const end = input.data() + input.size();
    for (char const* c = start; c != end; ++c) {
        if (*c == '\n') {
            _handleLine({start, static_cast<size_t>(c - start)});
            start = c + 1;
        }
    }
};

bool up::ReconClient::start(IOLoop& loop, zstring_view projectPath) {
    UP_ASSERT(!projectPath.empty());

    const char* const args[] = {"recon", "-project", projectPath.c_str(), "-server", nullptr};

    _sink = loop.createPipe();
    _source = loop.createPipe();

    IOProcessConfig config{
        .process = "recon",
        .args = args,
        .input = &_sink,
        .output = &_source,
    };

    _process = loop.createProcess();
    auto const ec = _process.spawn(config);

    if (ec != 0) {
        s_logger.error("Failed to start recon: {}", loop.errorString(ec));
        return false;
    }

    _source.startRead([this](auto input) { _onRead(input); });

    s_logger.info("Started recon PID={}", _process.pid());

    return true;
}

void up::ReconClient::stop() {
    _source.reset();
    _sink.reset();

    if (_process) {
        _process.terminate(true);
        _process.reset();
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

    auto const str = doc.dump();

    _sink.write({str.data(), str.size()});

    return true;
}
