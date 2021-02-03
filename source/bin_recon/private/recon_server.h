// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/concurrent_queue.h"
#include "potato/runtime/logger.h"
#include "potato/spud/delegate.h"

#include <thread>

namespace up::recon {
    class ReconServer {
    public:
        // Note: these will all be called on the recon server thread!
        using ImportHandler = delegate<void(schema::ReconImportMessage const&)>;
        using ImportAllHandler = delegate<void(schema::ReconImportAllMessage const&)>;
        using DisconnectHandler = delegate<void()>;

        explicit ReconServer(Logger& logger);
        ~ReconServer();

        void listenImport(ImportHandler handler);
        void listenImportAll(ImportAllHandler handler);
        void listenDisconnect(DisconnectHandler handler);

        bool sendLog(schema::ReconLogMessage const& msg);
        bool sendManifest(schema::ReconManifestMessage const& msg);

        bool start();

    private:
        static void _threadMain(ReconServer* server);
        bool _send(string_view encodedMsg);

        std::thread _ioThread;
        Logger _logger;
        ImportHandler _importHandler;
        ImportAllHandler _importAllHandler;
        DisconnectHandler _disconnectHandler;
    };
} // namespace up::recon
