// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"
#include "recon_messages_schema.h"

#include <reproc++/reproc.hpp>
#include <atomic>
#include <thread>

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

        bool UP_RECON_API start(Project& project);
        void UP_RECON_API stop();

        bool UP_RECON_API hasUpdatedAssets() noexcept;

        template <typename MessageT>
        bool sendMessage(MessageT const& msg) {
            return _sendRaw(reflex::getSchema<MessageT>(), &msg);
        }

    private:
        struct ReprocSink;

        bool UP_RECON_API _sendRaw(reflex::Schema const& schema, void const* object);
        void _handle(schema::ReconLogMessage const& msg);
        void _handle(schema::ReconManifestMessage const& msg);

        reproc::process _process;
        std::thread _thread;
        std::atomic_bool _staleAssets = false;

        friend ReprocSink;
    };
} // namespace up
