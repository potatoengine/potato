// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include "grimm/foundation/string_view.h"
#include "converters/convert_hlsl.h"
#include "converters/convert_copy.h"
#include "converters/convert_json.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <iostream>
#include <fstream>

gm::recon::ConverterApp::ConverterApp() : _programName("recon") {}
gm::recon::ConverterApp::~ConverterApp() = default;

bool gm::recon::ConverterApp::run(span<char const*> args) {
    if (!parseArguments(_config, args)) {
        std::cerr << "Failed to parse arguments\n";
        return false;
    }

    if (!_config.configFilePath.empty()) {
        if (!parseConfigFile(_config, _config.configFilePath)) {
        }
    }

    registerConverters();

    auto sources = collectSourceFiles();

    if (sources.empty()) {
        std::cerr << "No source files found\n";
        return false;
    }

    if (_config.sourceFolderPath.empty()) {
        std::cerr << "Source directory must be specified.\n";
        return false;
    }
    if (_config.destinationFolderPath.empty()) {
        std::cerr << "Destination directory must be specified.\n";
        return false;
    }
    if (_config.cacheFolderPath.empty()) {
        std::cerr << "Cache directory must be specified.\n";
        return false;
    }

    std::cout << "Source: " << _config.sourceFolderPath << "\n";
    std::cout << "Destination: " << _config.destinationFolderPath << "\n";
    std::cout << "Cache: " << _config.cacheFolderPath << "\n";

    if (!std::filesystem::is_directory(_config.destinationFolderPath)) {
        std::error_code rs;
        if (!std::filesystem::create_directories(_config.destinationFolderPath, rs)) {
            std::cerr << "Failed to create `" << _config.destinationFolderPath << "': " << rs.message() << '\n';
            return false;
        }
    }

    if (!std::filesystem::is_directory(_config.cacheFolderPath)) {
        std::error_code rs;
        if (!std::filesystem::create_directories(_config.cacheFolderPath, rs)) {
            std::cerr << "Failed to create `" << _config.cacheFolderPath << "': " << rs.message() << '\n';
            return false;
        }
    }

    if (!convertFiles(std::move(sources))) {
        std::cerr << "Conversion failed\n";
        return false;
    }

    auto libraryPath = _config.destinationFolderPath / "library$.json";
    std::ofstream libraryStream(libraryPath);
    if (!_library.serialize(libraryStream)) {
        std::cerr << "Failed to write asset library\n";
        return false;
    }
    libraryStream.close();

    return true;
}

void gm::recon::ConverterApp::registerConverters() {
#if GM_GPU_ENABLE_D3D12
    _converters.push_back({[](std::filesystem::path const& path) { return path.extension() == ".hlsl"; },
                           make_box<HlslConverter>()});
#endif
    _converters.push_back({[](std::filesystem::path const& path) { return path.extension() == ".json"; },
                           make_box<JsonConverter>()});
    _converters.push_back({[](std::filesystem::path const& path) { return path.extension() == ".png"; },
                           make_box<CopyConverter>()});
}

bool gm::recon::ConverterApp::convertFiles(vector<std::filesystem::path> files) {
    bool failed = false;

    for (auto const& path : files) {
        std::cout << "Processing `" << path << "`\n";

        auto assetPath = path.generic_string();
        auto assetId = _library.pathToAssetId(string_view(assetPath));
        auto record = _library.findRecord(assetId);

        Converter* converter = findConverter(path);
        if (converter == nullptr) {
            failed = true;
            std::cerr << "Converter not found for `" << path << "'\n";
            continue;
        }

        Context context(path, _config.sourceFolderPath, _config.destinationFolderPath);
        if (!converter->convert(context)) {
            failed = true;
            std::cerr << "Failed conversion for `" << path << "'\n";
            continue;
        }

        AssetRecord newRecord;
        newRecord.assetId = assetId;
        newRecord.path = string_view(assetPath);
        newRecord.contentHash = 0;
        _library.insertRecord(std::move(newRecord));
    }

    return !failed;
}

auto gm::recon::ConverterApp::findConverter(path const& path) const -> Converter* {
    for (auto const& mapping : _converters) {
        if (mapping.predicate(path)) {
            return mapping.conveter.get();
        }
    }

    return nullptr;
}

auto gm::recon::ConverterApp::collectSourceFiles() -> vector<std::filesystem::path> {

    if (!std::filesystem::is_directory(_config.sourceFolderPath)) {
        std::cerr << "`" << _config.sourceFolderPath << "' does not exist or is not a directory\n";
        return {};
    }

    vector<std::filesystem::path> files;

    for (auto&& path : std::filesystem::recursive_directory_iterator(_config.sourceFolderPath)) {
        if (path.is_regular_file()) {
            files.push_back(std::filesystem::relative(std::move(path).path(), _config.sourceFolderPath));
        }
    }

    return files;
}
