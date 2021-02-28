// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_protocol.h"

#include "potato/runtime/io_loop.h"
#include "potato/spud/delegate.h"

namespace up::reflex {
    struct Schema;
    template <typename T>
    Schema const& getSchema();
} // namespace up::reflex

namespace up {
    class Project;

    class ReconClient {
    public:
        UP_RECON_API ReconClient();
        ~ReconClient() { stop(); }

        bool UP_RECON_API start(IOLoop& loop, zstring_view projectPath);
        void UP_RECON_API stop();

        UP_RECON_API void onManifestChange(delegate<void()> callback);

        template <typename MessageT>
        bool sendMessage(zstring_view name, MessageT const& msg) {
            return _handler.send(name, msg, _sink);
        }

    private:
        ReconProtocol _handler;
        IOProcess _process;
        IOPipe _sink;
        IOPipe _source;

        delegate<void()> _onManifest;
    };
} // namespace up
