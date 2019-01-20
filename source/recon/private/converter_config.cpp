// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/recon/converter_config.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <iostream>
#include <fstream>

bool gm::recon::parseArguments(ConverterConfig& config, span<char const*> args) {
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
                std::cerr << "Unexpected option: " << arg << '\n';
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
                std::cerr << "Unknown option: " << arg << '\n';
                return false;
            }
            continue;
        }

        switch (argMode) {
        case ArgNone:
            std::cerr << "Unexpected value: " << arg << '\n';
            return false;
        case ArgSourceFolder:
            config.sourceFolderPath = arg;
            argMode = ArgNone;
            break;
        case ArgDestinationFolder:
            config.destinationFolderPath = arg;
            argMode = ArgNone;
            break;
        case ArgCacheFolder:
            config.cacheFolderPath = arg;
            argMode = ArgNone;
            break;
        case ArgConfig:
            if (!parseConfigFile(config, arg)) {
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

bool gm::recon::parseConfigFile(ConverterConfig& config, zstring_view path) {
    std::fstream file(path.c_str(), std::ios_base::in);
    if (!file) {
        std::cerr << "Failed to open `" << path << "'\n";
        return false;
    }

    std::string text;
    file.seekg(0, std::ios::end);
    text.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(text.data(), text.size());

    return parseConfigString(config, std::string_view(text));
}

bool gm::recon::parseConfigString(ConverterConfig& config, string_view json) {
    rapidjson::Document doc;

    doc.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(json.data(), json.size());

    if (doc.HasParseError()) {
        std::cerr << "Failed to parse JSON: " << doc.GetParseError() << '\n';
        return false;
    }
    if (!doc.IsObject()) {
        return false;
    }

    if (doc.HasMember("sourceDir")) {
        config.sourceFolderPath = doc["sourceDir"].GetString();
    }
    if (doc.HasMember("destDir")) {
        config.destinationFolderPath = doc["destDir"].GetString();
    }
    if (doc.HasMember("cacheDir")) {
        config.cacheFolderPath = doc["cacheDir"].GetString();
    }
    if (doc.HasMember("deleteStale")) {
        config.deleteStale = doc["deleteStale"].GetBool();
    }
    return true;
}
