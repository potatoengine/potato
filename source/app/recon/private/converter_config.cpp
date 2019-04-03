// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "potato/recon/converter_config.h"
#include "potato/foundation/string_view.h"
#include "potato/foundation/zstring_view.h"
#include "potato/filesystem/filesystem.h"
#include "potato/filesystem/stream_util.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <iostream>

bool up::recon::parseArguments(ConverterConfig& config, span<char const*> args, fs::FileSystem& fileSystem, spdlog::logger& logger) {
    if (args.empty()) {
        return false;
    }

    [[maybe_unused]] auto program = args.front();
    args.pop_front();

    enum {
        ArgNone,
        ArgSourceFolder,
        ArgDestinationFolder,
        ArgCacheFolder,
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
            else if (name == "dest") {
                argMode = ArgDestinationFolder;
            }
            else if (name == "cache") {
                argMode = ArgCacheFolder;
            }
            else if (name == "config") {
                argMode = ArgConfig;
            }
            else if (name == "delete") {
                config.deleteStale = true;
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
        case ArgDestinationFolder:
            config.destinationFolderPath = string(arg);
            argMode = ArgNone;
            break;
        case ArgCacheFolder:
            config.cacheFolderPath = string(arg);
            argMode = ArgNone;
            break;
        case ArgConfig:
            if (!parseConfigFile(config, fileSystem, arg, logger)) {
                return false;
            }
            argMode = ArgNone;
            break;
        }
    }

    if (argMode != ArgNone) {
        std::cerr << "Value expected\n";
        return false;
    }

    return true;
}

bool up::recon::parseConfigFile(ConverterConfig& config, fs::FileSystem& fileSystem, zstring_view path, spdlog::logger& logger) {
    auto stream = fileSystem.openRead(path, fs::FileOpenMode::Text);
    if (!stream) {
        logger.error("Failed to open `{}'", path.c_str());
        return false;
    }

    string text;
    if (fs::readText(stream, text) != fs::Result::Success) {
        logger.error("Failed to read `{}'", path.c_str());
        return false;
    }

    return parseConfigString(config, text, path, logger);
}

bool up::recon::parseConfigString(ConverterConfig& config, string_view json, zstring_view filename, spdlog::logger& logger) {
    rapidjson::Document doc;

    doc.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(json.data(), json.size());

    if (doc.HasParseError()) {
        logger.error("Failed to parse file `{}': {}", filename, doc.GetParseError());
        return false;
    }
    if (!doc.IsObject()) {
        return false;
    }

    if (doc.HasMember("sourceDir")) {
        config.sourceFolderPath = string(doc["sourceDir"].GetString());
    }
    if (doc.HasMember("destDir")) {
        config.destinationFolderPath = string(doc["destDir"].GetString());
    }
    if (doc.HasMember("cacheDir")) {
        config.cacheFolderPath = string(doc["cacheDir"].GetString());
    }
    if (doc.HasMember("deleteStale")) {
        config.deleteStale = doc["deleteStale"].GetBool();
    }
    return true;
}
