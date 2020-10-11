// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_config.h"

#include "potato/recon/config_schema.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/logger.h"
#include "potato/runtime/stream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/zstring_view.h"

#include <nlohmann/json.hpp>

bool up::recon::parseArguments(ReconConfig& config, span<char const*> args, Logger& logger) {
    if (args.empty()) {
        return false;
    }

    [[maybe_unused]] auto program = args.front();
    args.pop_front();

    enum {
        ArgNone,
        ArgSourceFolder,
        ArgConfig,
    } argMode = ArgNone;

    for (zstring_view arg : args) {
        if (!arg.empty() && arg.front() == '-') {
            if (argMode != ArgNone) {
                logger.error("Unexpected option: {}", arg.c_str());
                return false;
            }

            auto name = arg.substr(1);
            if (name == "source") {
                argMode = ArgSourceFolder;
            }
            else if (name == "config") {
                argMode = ArgConfig;
            }
            else {
                logger.error("Unknown option: {}", arg.c_str());
                return false;
            }
            continue;
        }

        switch (argMode) {
            case ArgNone:
                logger.error("Unexpected value: {}", arg.c_str());
                return false;
            case ArgSourceFolder:
                config.sourceFolderPath = string(arg);
                argMode = ArgNone;
                break;
            case ArgConfig:
                if (!parseConfigFile(config, arg, logger)) {
                    return false;
                }
                argMode = ArgNone;
                break;
        }
    }

    switch (argMode) {
        case ArgNone:
            return true;
        case ArgSourceFolder:
            logger.error("No value provided after `-source' argument");
            return false;
        case ArgConfig:
            logger.error("No value provided after `-config' argument");
            return false;
        default:
            logger.error("No value provided");
            return false;
    }
}

bool up::recon::parseConfigFile(ReconConfig& config, zstring_view path, Logger& logger) {
    auto stream = fs::openRead(path, fs::OpenMode::Text);
    if (!stream) {
        logger.error("Failed to open `{}'", path.c_str());
        return false;
    }

    string text;
    if (readText(stream, text) != IOResult::Success) {
        logger.error("Failed to read `{}'", path.c_str());
        return false;
    }

    return parseConfigString(config, text, path, logger);
}

bool up::recon::parseConfigString(ReconConfig& config, string_view json, zstring_view filename, Logger& logger) {
    auto jsonRoot = nlohmann::json::parse(json.begin(), json.end(), nullptr, false);
    if (!jsonRoot.is_object()) {
        logger.error("Failed to parse file `{}': {}", filename, "unknown parse error");
        return false;
    }

    jsonRoot.get_to(config);
    return true;
}
