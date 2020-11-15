// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include <reproc++/reproc.hpp>

namespace up {
    class Project;
}

namespace up::shell {
    class ReconClient {
    public:
        ~ReconClient() { stop(); }

        bool start(Project& project);
        void stop();

    private:
        reproc::process _process;
    };
} // namespace up::shell
