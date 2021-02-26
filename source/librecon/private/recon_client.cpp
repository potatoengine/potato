// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/logger.h"

#include <nlohmann/json.hpp>

static up::Logger s_logger("ReconClient"); // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

static void onAlloc(uv_handle_t* handle, size_t len, uv_buf_t* buf) {
    buf->base = (char*)malloc(len); // NOLINT
    buf->len = static_cast<unsigned long>(len);
}

static void onClose(uv_handle_t* handle) {
    delete handle; // NOLINT
}

static void onWrite(uv_write_t* req, int status) {
    free(req->data); // NOLINT
    delete req; // NOLINT
}

void up::ReconClient::_handleLine(up::ReconClient& client, up::string_view line) {
    using namespace up::schema;

    nlohmann::json doc = nlohmann::json::parse(line, nullptr, false, true);

    auto handleLog = [&client](ReconLogMessage const& msg) {
        client._handle(msg);
    };
    auto handleManifest = [&client](ReconManifestMessage const& msg) {
        client._handle(msg);
    };

    decodeReconMessage<ReconLogMessage>(doc, handleLog) ||
        decodeReconMessage<ReconManifestMessage>(doc, handleManifest);
}

void up::ReconClient::_onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf) {
    auto* client = static_cast<ReconClient*>(stream->data);

    char const* start = buf->base;
    char const* const end = start + buf->len;

    // FIXME: this assumes we're getting whole lines, which is not guaranteed!
    for (char const* c = start; c != end; ++c) {
        if (*c == '\n') {
            client->_handleLine(*client, {start, static_cast<size_t>(c - start)});
            start = c + 1;
        }
    }

    free(buf->base); // NOLINT
};

bool up::ReconClient::start(uv_loop_t* loop, zstring_view projectPath) {
    UP_ASSERT(loop != nullptr);
    UP_ASSERT(!projectPath.empty());

    stop();

    const char* const args[] = {"recon", "-project", projectPath.c_str(), "-server", nullptr};

    _sink.reset(new uv_pipe_t);
    uv_pipe_init(loop, _sink.get(), 0);
    _sink->data = this;

    _source.reset(new uv_pipe_t);
    uv_pipe_init(loop, _source.get(), 0);
    _source->data = this;

    uv_stdio_container_t stdio[] = {
        {.flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_READABLE_PIPE),
         .data = {.stream = reinterpret_cast<uv_stream_t*>(_sink.get())}},
        {.flags = static_cast<uv_stdio_flags>(UV_CREATE_PIPE | UV_WRITABLE_PIPE),
         .data = {.stream = reinterpret_cast<uv_stream_t*>(_source.get())}},
        {.flags = UV_IGNORE}
    };
    uv_process_options_t options = {
        .file = "recon",
        .args = (char**)args, // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)
        .flags = UV_PROCESS_WINDOWS_HIDE,
        .stdio_count = sizeof(stdio) / sizeof(stdio[0]),
        .stdio = stdio};

    _process.reset(new uv_process_t);
    auto const ec = uv_spawn(loop, _process.get(), &options);
    _process->data = this;

    if (ec != 0) {
        s_logger.error("Failed to start recon: {}", uv_strerror(ec));
        return false;
    }

    uv_read_start(reinterpret_cast<uv_stream_t*>(_source.get()), &onAlloc, &_onRead);

    s_logger.info("Started recon PID={}", _process->pid);

    return true;
}

void up::ReconClient::stop() {
    if (_source != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t*>(_source.release()), &onClose);
    }
    if (_sink != nullptr) {
        uv_close(reinterpret_cast<uv_handle_t*>(_sink.release()), &onClose);
    }
    if (_process != nullptr) {
        uv_process_kill(_process.get(), SIGTERM);
        uv_close(reinterpret_cast<uv_handle_t*>(_process.release()), &onClose);
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
    int const length = static_cast<int>(str.size()) + 2 /* CR+LF */;
    const uv_buf_t buf = uv_buf_init((char*)malloc(length), length); // NOLINT

    std::memcpy(buf.base, str.data(), str.size());
    buf.base[str.size()] = '\r';
    buf.base[str.size() + 1] = '\n';

    auto* req = new uv_write_t;
    uv_write(req, reinterpret_cast<uv_stream_t*>(_sink.get()), &buf, 1, onWrite);
    req->data = buf.base;

    return true;
}
