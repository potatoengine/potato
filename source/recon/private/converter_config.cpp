// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "grimm/recon/converter_config.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/zstring_view.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream_util.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <iostream>

bool gm::recon::parseArguments(ConverterConfig& config, span<char const*> args, fs::FileSystem& fileSystem) {
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
                std::cerr << "Unexpected option: " << arg.c_str() << '\n';
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
                std::cerr << "Unknown option: " << arg.c_str() << '\n';
                return false;
            }
            continue;
        }

        switch (argMode) {
        case ArgNone:
            std::cerr << "Unexpected value: " << arg.c_str() << '\n';
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
            if (!parseConfigFile(config, fileSystem, arg)) {
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

bool gm::recon::parseConfigFile(ConverterConfig& config, fs::FileSystem& fileSystem, zstring_view path) {
    auto stream = fileSystem.openRead(path, fs::FileOpenMode::Text);
    if (!stream) {
        std::cerr << "Failed to open `" << path.c_str() << "'\n";
        return false;
    }

    string text;
    if (fs::readText(stream, text) != fs::Result::Success) {
        std::cerr << "Failed to read `" << path.c_str() << "'\n";
        return false;
    }

    return parseConfigString(config, text, path);
}

bool gm::recon::parseConfigString(ConverterConfig& config, string_view json, zstring_view filename) {
    rapidjson::Document doc;

    doc.Parse<rapidjson::kParseCommentsFlag | rapidjson::kParseTrailingCommasFlag | rapidjson::kParseNanAndInfFlag>(json.data(), json.size());

    if (doc.HasParseError()) {
        std::cerr << "Failed to parse file `" << filename.c_str() << "': " << doc.GetParseError() << '\n';
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
