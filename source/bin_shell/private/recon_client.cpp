// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_client.h"

#include "potato/tools/project.h"
#include "potato/runtime/logger.h"

static up::Logger s_logger("ReconClient");

bool up::shell::ReconClient::start(Project& project) {
    stop();

    reproc::options options = {

    };

    const char* const args[] = {"recon", "-project", project.projectFilePath().c_str(), "-server", nullptr};
    auto const ec = _process.start(args);
    if (ec) {
        s_logger.error("Failed to start recon: {}", ec.message());
        return false;
    }

    auto [pid, _] = _process.pid();
    s_logger.info("Started recon PID={}", pid);

    return true;
}

void up::shell::ReconClient::stop() {
    _process.terminate();
}
