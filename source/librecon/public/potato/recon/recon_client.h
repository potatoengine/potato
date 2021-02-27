// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include "potato/runtime/io_loop.h"

#include <atomic>

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

        bool UP_RECON_API start(IOLoop& loop, zstring_view projectPath);
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
        void _onRead(span<char> input);
        void _handleLine(string_view line);

        IOProcess _process;
        IOPipe _sink;
        IOPipe _source;
        std::atomic_bool _staleAssets = false;
    };
} // namespace up
