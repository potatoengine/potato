// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include "potato/spud/concepts.h"

#include <reproc++/reproc.hpp>
#include <atomic>
#include <thread>

namespace up {
    class Project;
}

namespace up::schema {
    struct ReconMessage;
}

namespace up::reflex {
    struct Schema;
    template <typename T>
    Schema const& getSchema();
} // namespace up::reflex

namespace up::shell {
    class ReconClient {
    public:
        ~ReconClient() { stop(); }

        bool UP_RECON_API start(Project& project);
        void UP_RECON_API stop();

        bool UP_RECON_API hasUpdatedAssets() noexcept;
        bool UP_RECON_API handleMessage(reflex::Schema const& schema, schema::ReconMessage const& msg);
        bool UP_RECON_API sendMessage(reflex::Schema const& schema, schema::ReconMessage const& msg);

        template <derived_from<schema::ReconMessage> MessageT>
        bool sendMessage(MessageT const& msg) {
            return sendMessage(reflex::getSchema<MessageT>(), msg);
        }

    private:
        struct ReprocSink;

        reproc::process _process;
        std::thread _thread;
        std::atomic_bool _staleAssets = false;

        friend ReprocSink;
    };
} // namespace up::shell
