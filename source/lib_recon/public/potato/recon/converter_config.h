// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "potato/spud/span.h"
#include "potato/spud/string.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"
#include "potato/spud/std_iostream.h"
#include "potato/runtime/logger.h"

namespace up {
    class FileSystem;
}

namespace up::recon {
    struct ConverterConfig {
        string sourceFolderPath;
        string destinationFolderPath;
        string cacheFolderPath;
        bool deleteStale = false;
    };

    bool parseArguments(ConverterConfig& config, span<char const*> args, FileSystem& fileSystem, Logger& logger);
    bool parseConfigFile(ConverterConfig& config, FileSystem& fileSystem, zstring_view path, Logger& logger);
    bool parseConfigString(ConverterConfig& config, string_view json, zstring_view filename, Logger& logger);
} // namespace up::recon
