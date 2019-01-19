// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include "grimm/foundation/string_view.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/library/asset_hashes.h"
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
        if (!parseConfigFile(_config, _config.configFilePath.c_str())) {
        }
    }

    registerConverters();

    fs::FileSystem fs;

    auto libraryPath = fs::path::join({string_view(_config.destinationFolderPath), "library$.json"});
    if (fs.fileExists(libraryPath.c_str())) {
        std::ifstream libraryReadStream(libraryPath);
        if (!libraryReadStream) {
            std::cerr << "Failed to open asset library `" << libraryPath << "'\n";
        }
        if (!_library.deserialize(libraryReadStream)) {
            std::cerr << "Failed to load asset library `" << libraryPath << "'\n";
        }
        std::cout << "Loaded asset library `" << libraryPath << "'\n";
    }

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

    if (!fs.directoryExists(_config.destinationFolderPath.c_str())) {
        if (!fs.createDirectories(_config.destinationFolderPath.c_str())) {
            std::cerr << "Failed to create `" << _config.destinationFolderPath << "'\n";
            return false;
        }
    }

    if (!fs.directoryExists(_config.cacheFolderPath.c_str())) {
        if (!fs.createDirectories(_config.cacheFolderPath.c_str())) {
            std::cerr << "Failed to create `" << _config.cacheFolderPath << "'\n";
            return false;
        }
    }

    if (!convertFiles(sources)) {
        std::cerr << "Conversion failed\n";
        return false;
    }

    std::ofstream libraryWriteStream(libraryPath);
    if (!_library.serialize(libraryWriteStream)) {
        std::cerr << "Failed to write asset library `" << libraryPath << "'\n";
        return false;
    }
    libraryWriteStream.close();

    return true;
}

void gm::recon::ConverterApp::registerConverters() {
#if GM_GPU_ENABLE_D3D12
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".hlsl"; },
                           make_box<HlslConverter>()});
#endif
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".json"; },
                           make_box<JsonConverter>()});
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".png"; },
                           make_box<CopyConverter>()});
}

bool gm::recon::ConverterApp::convertFiles(vector<std::string> const& files) {
    bool failed = false;

    for (auto const& path : files) {
        std::cout << "Processing `" << path << "`\n";

        auto assetId = _library.pathToAssetId(string_view(path));
        auto record = _library.findRecord(assetId);

        auto osPath = fs::path::join({_config.sourceFolderPath.c_str(), path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

        Converter* converter = findConverter(string_view(path));
        if (converter == nullptr) {
            failed = true;
            std::cerr << "Converter not found for `" << path << "'\n";
            continue;
        }

        bool upToDate = record != nullptr && isUpToDate(*record, contentHash, *converter);
        if (upToDate) {
            std::cout << "Asset `" << path << "' is up-to-date\n";
            continue;
        }

        std::cout << "Asset `" << path << "' requires import (" << converter->name() << ")\n";

        AssetImportRecord newRecord;
        newRecord.assetId = assetId;
        newRecord.path = path.c_str();
        newRecord.contentHash = contentHash;
        newRecord.category = AssetCategory::Source;
        newRecord.importerName = converter->name();
        newRecord.importerRevision = converter->revision();
        _library.insertRecord(std::move(newRecord));

        Context context(path.c_str(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str());
        if (!converter->convert(context)) {
            failed = true;
            std::cerr << "Failed conversion for `" << path << "'\n";
            continue;
        }
    }

    return !failed;
}

bool gm::recon::ConverterApp::isUpToDate(AssetImportRecord const& record, gm::uint64 contentHash, Converter const& converter) const noexcept {
    return record.contentHash == contentHash &&
           string_view(record.importerName) == converter.name() &&
           record.importerRevision == converter.revision();
}

auto gm::recon::ConverterApp::findConverter(string_view path) const -> Converter* {
    for (auto const& mapping : _converters) {
        if (mapping.predicate(path)) {
            return mapping.conveter.get();
        }
    }

    return nullptr;
}

auto gm::recon::ConverterApp::collectSourceFiles() -> vector<std::string> {
    fs::FileSystem fs;

    if (!fs.directoryExists(_config.sourceFolderPath.c_str())) {
        std::cerr << "`" << _config.sourceFolderPath << "' does not exist or is not a directory\n";
        return {};
    }

    vector<std::string> files;

    auto cb = [&files](fs::FileInfo const& info) -> fs::EnumerateResult {
        if (info.type == fs::FileType::Regular) {
            files.push_back(info.path);
        }
        else if (info.type == fs::FileType::Directory) {
            return fs::EnumerateResult::Recurse;
        }
        return fs::EnumerateResult::Continue;
    };
    fs.enumerate(_config.sourceFolderPath.c_str(), cb, fs::EnumerateOptions::None);

    return files;
}
