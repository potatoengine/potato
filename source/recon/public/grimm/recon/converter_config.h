// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/span.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include <string>

namespace gm::recon {
    struct ConverterConfig {
        std::string configFilePath;
        std::string sourceFolderPath;
        std::string destinationFolderPath;
        std::string cacheFolderPath;
    };

    bool parseArguments(ConverterConfig& config, span<char const*> args);
    bool parseConfigFile(ConverterConfig& config, zstring_view path);
    bool parseConfigString(ConverterConfig& config, string_view json);
} // namespace gm::recon
