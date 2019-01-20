// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#pragma once

#include "grimm/foundation/span.h"
#include "grimm/foundation/string_blob.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include <string>

namespace gm::recon {
    struct ConverterConfig {
        string configFilePath;
        string sourceFolderPath;
        string destinationFolderPath;
        string cacheFolderPath;
        bool deleteStale = false;
    };

    bool parseArguments(ConverterConfig& config, span<char const*> args);
    bool parseConfigFile(ConverterConfig& config, zstring_view path);
    bool parseConfigString(ConverterConfig& config, string_view json);
} // namespace gm::recon
