// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/io_loop.h"
#include "potato/runtime/logger.h"
#include "potato/spud/delegate.h"

namespace up::recon {
    class ReconServer {
    public:
        // Note: these will all be called on the recon server thread!
        using ImportHandler = delegate<void(schema::ReconImportMessage const&)>;
        using ImportAllHandler = delegate<void(schema::ReconImportAllMessage const&)>;
        using DisconnectHandler = delegate<void()>;

        ReconServer(IOLoop& loop, Logger& logger);
        ~ReconServer();

        void listenImport(ImportHandler handler);
        void listenImportAll(ImportAllHandler handler);
        void listenDisconnect(DisconnectHandler handler);

        bool sendLog(schema::ReconLogMessage const& msg);
        bool sendManifest(schema::ReconManifestMessage const& msg);

        bool start();

    private:
        bool _send(string_view encodedMsg);

        Logger _logger;
        IOPipe _sink;
        IOPipe _source;
        ImportHandler _importHandler;
        ImportAllHandler _importAllHandler;
        DisconnectHandler _disconnectHandler;
    };
} // namespace up::recon
