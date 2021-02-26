// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include <atomic>

#define NOMINMAX
#include <uv.h>

namespace up::reflex {
    struct Schema;
    template <typename T>
    Schema const& getSchema();
} // namespace up::reflex

namespace up {
    class Project;

    class ReconClient {
    public:
        ~ReconClient() { stop(); }

        bool UP_RECON_API start(uv_loop_t* loop, zstring_view projectPath);
        void UP_RECON_API stop();

        bool UP_RECON_API hasUpdatedAssets() noexcept;

        template <typename MessageT>
        bool sendMessage(MessageT const& msg) {
            return _sendRaw(reflex::getSchema<MessageT>(), &msg);
        }

    private:
        bool UP_RECON_API _sendRaw(reflex::Schema const& schema, void const* object);
        void _handle(schema::ReconLogMessage const& msg);
        void _handle(schema::ReconManifestMessage const& msg);
        static void _handleLine(ReconClient& client, string_view line);
        static void _onRead(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf);

        box<uv_process_t> _process;
        box<uv_pipe_t> _sink;
        box<uv_pipe_t> _source;
        std::atomic_bool _staleAssets = false;
    };
} // namespace up
