// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

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

        bool start(Project& project);
        void stop();

        bool hasUpdatedAssets() noexcept;

    private:
        struct ReprocSink;

        reproc::process _process;
        std::thread _thread;
        std::atomic_bool _staleAssets;

        friend ReprocSink;
    };
} // namespace up::shell
