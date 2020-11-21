// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "_export.h"

#include <reproc++/reproc.hpp>
#include <atomic>
#include <thread>

namespace up {
    class Project;
}

namespace up::shell {
    class ReconClient {
    public:
        ~ReconClient() { stop(); }

        bool UP_RECON_API start(Project& project);
        void UP_RECON_API stop();

        bool UP_RECON_API hasUpdatedAssets() noexcept;

    private:
        struct ReprocSink;

        reproc::process _process;
        std::thread _thread;
        std::atomic_bool _staleAssets = false;

        friend ReprocSink;
    };
} // namespace up::shell
