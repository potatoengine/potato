// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#pragma once

#include "potato/runtime/logger.h"
#include "potato/spud/span.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/vector.h"
#include "potato/spud/zstring_view.h"

namespace up::recon {
    struct ReconConfig {
        string sourceFolderPath;

        struct ImportMapping {
            string pattern;
            string importer;
        };
        vector<ImportMapping> mapping;
    };

    bool parseArguments(ReconConfig& config, span<char const*> args, Logger& logger);
    bool parseConfigFile(ReconConfig& config, zstring_view path, Logger& logger);
    bool parseConfigString(ReconConfig& config, string_view json, zstring_view filename, Logger& logger);
} // namespace up::recon
