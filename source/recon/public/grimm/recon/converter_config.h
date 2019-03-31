// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/span.h"
#include "grimm/foundation/gmstring.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/foundation/std_iostream.h"
#include <string>
#include <spdlog/logger.h>
#include <spdlog/fmt/ostr.h>

namespace gm::fs {
    class FileSystem;
}

namespace gm::recon {
    struct ConverterConfig {
        string sourceFolderPath;
        string destinationFolderPath;
        string cacheFolderPath;
        bool deleteStale = false;
    };

    bool parseArguments(ConverterConfig& config, span<char const*> args, fs::FileSystem& fileSystem, spdlog::logger& logger);
    bool parseConfigFile(ConverterConfig& config, fs::FileSystem& fileSystem, zstring_view path, spdlog::logger& logger);
    bool parseConfigString(ConverterConfig& config, string_view json, zstring_view filename, spdlog::logger& logger);
} // namespace gm::recon
