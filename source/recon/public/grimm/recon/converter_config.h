// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/span.h"
#include "grimm/foundation/string_view.h"
#include <filesystem>

namespace gm::recon {
    struct ConverterConfig {
        std::filesystem::path configFilePath;
        std::filesystem::path sourceFolderPath;
        std::filesystem::path destinationFolderPath;
        std::filesystem::path cacheFolderPath;
    };

    bool parseArguments(ConverterConfig& config, span<char const*> args);
    bool parseConfigFile(ConverterConfig& config, std::filesystem::path const& path);
    bool parseConfigString(ConverterConfig& config, string_view json);
} // namespace gm::recon
