// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/recon/recon_protocol.h"
#include "potato/runtime/io_loop.h"
#include "potato/runtime/logger.h"
#include "potato/spud/delegate.h"

namespace up::recon {
    class ReconServer : public ReconProtocol {
    public:
        using DisconnectHandler = delegate<void()>;

        explicit ReconServer(Logger& logger);
        ~ReconServer() { stop(); }

        void onDisconnect(DisconnectHandler handler);

        bool start(IOLoop& loop);
        void stop();

    private:
        IOStream& sink() override { return _sink; }

        Logger _logger;
        IOStream _sink;
        IOStream _source;
        DisconnectHandler _disconnectHandler;
    };
} // namespace up::recon
