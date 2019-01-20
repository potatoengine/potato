// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include "grimm/foundation/string_view.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/library/hash_cache.h"
#include "converters/convert_hlsl.h"
#include "converters/convert_copy.h"
#include "converters/convert_json.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <iostream>
#include <fstream>
#include <set>

gm::recon::ConverterApp::ConverterApp() : _programName("recon"), _hashes(_fileSystem) {}
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

    auto libraryPath = fs::path::join({string_view(_config.destinationFolderPath), "library$.json"});
    _outputs.push_back("library$.json");
    if (_fileSystem.fileExists(libraryPath.c_str())) {
        std::ifstream libraryReadStream = _fileSystem.openRead(libraryPath.c_str(), fs::FileOpenMode::Text);
        if (!libraryReadStream) {
            std::cerr << "Failed to open asset library `" << libraryPath << "'\n";
        }
        if (!_library.deserialize(libraryReadStream)) {
            std::cerr << "Failed to load asset library `" << libraryPath << "'\n";
        }
        std::cout << "Loaded asset library `" << libraryPath << "'\n";
    }

    auto hashCachePath = fs::path::join({string_view(_config.destinationFolderPath), "hashes$.json"});
    _outputs.push_back("hashes$.json");
    if (_fileSystem.fileExists(hashCachePath.c_str())) {
        std::ifstream hashesReadStream = _fileSystem.openRead(hashCachePath.c_str(), fs::FileOpenMode::Text);
        if (!hashesReadStream) {
            std::cerr << "Failed to open hash cache `" << hashCachePath << "'\n";
        }
        if (!_hashes.deserialize(hashesReadStream)) {
            std::cerr << "Failed to load hash cache `" << hashCachePath << "'\n";
        }
        std::cout << "Loaded hash cache `" << hashCachePath << "'\n";
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

    if (!_fileSystem.directoryExists(_config.destinationFolderPath.c_str())) {
        if (!_fileSystem.createDirectories(_config.destinationFolderPath.c_str())) {
            std::cerr << "Failed to create `" << _config.destinationFolderPath << "'\n";
            return false;
        }
    }

    if (!_fileSystem.directoryExists(_config.cacheFolderPath.c_str())) {
        if (!_fileSystem.createDirectories(_config.cacheFolderPath.c_str())) {
            std::cerr << "Failed to create `" << _config.cacheFolderPath << "'\n";
            return false;
        }
    }

    bool success = convertFiles(sources);
    if (!success) {
        std::cerr << "Conversion failed\n";
    }

    std::ofstream hashesWriteStream = _fileSystem.openWrite(hashCachePath.c_str(), fs::FileOpenMode::Text);
    if (!_hashes.serialize(hashesWriteStream)) {
        std::cerr << "Failed to write hash cache `" << hashCachePath << "'\n";
        return false;
    }
    hashesWriteStream.close();

    std::ofstream libraryWriteStream = _fileSystem.openWrite(libraryPath.c_str(), fs::FileOpenMode::Text);
    if (!_library.serialize(libraryWriteStream)) {
        std::cerr << "Failed to write asset library `" << libraryPath << "'\n";
        return false;
    }
    libraryWriteStream.close();

    if (success) {
        deleteUnusedFiles(_outputs, !_config.deleteStale);
    }

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

bool gm::recon::ConverterApp::convertFiles(vector<string> const& files) {
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

        bool upToDate = record != nullptr && isUpToDate(*record, contentHash, *converter) && isUpToDate(record->sourceDependencies);
        if (upToDate) {
            std::cout << "Asset `" << path << "' is up-to-date\n";
            for (auto const& rec : record->outputs) {
                _outputs.push_back(rec.path);
            }
            continue;
        }

        std::cout << "Asset `" << path << "' requires import (" << converter->name() << ")\n";

        Context context(path.c_str(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str());
        if (!converter->convert(context)) {
            failed = true;
            std::cerr << "Failed conversion for `" << path << "'\n";
            continue;
        }

        // range insert won't work - explicit copy fails
        for (auto const& output : context.outputs()) {
            _outputs.push_back(string(output));
        }

        AssetImportRecord newRecord;
        newRecord.assetId = assetId;
        newRecord.path = string(path);
        newRecord.contentHash = contentHash;
        newRecord.category = AssetCategory::Source;
        newRecord.importerName = string(converter->name());
        newRecord.importerRevision = converter->revision();

        for (auto const& sourceDepPath : context.sourceDependencies()) {
            auto osPath = fs::path::join({_config.sourceFolderPath.c_str(), sourceDepPath.c_str()});
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            newRecord.sourceDependencies.push_back(AssetDependencyRecord{
                string(sourceDepPath),
                contentHash});
        }

        for (auto const& outputPath : context.outputs()) {
            auto osPath = fs::path::join({_config.destinationFolderPath.c_str(), outputPath.c_str()});
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            newRecord.outputs.push_back(AssetOutputRecord{
                string(outputPath),
                contentHash});
        }

        _library.insertRecord(std::move(newRecord));
    }

    return !failed;
}

bool gm::recon::ConverterApp::deleteUnusedFiles(vector<string> const& files, bool dryRun) {
    std::set<string> keepFiles(files.begin(), files.end());
    std::set<string> foundFiles;
    auto cb = [&foundFiles](fs::FileInfo const& info) {
        if (info.type == fs::FileType::Regular) {
            foundFiles.insert(string(info.path));
        }
        return fs::EnumerateResult::Recurse;
    };
    _fileSystem.enumerate(_config.destinationFolderPath.c_str(), cb);

    vector<string> deleteFiles;
    std::set_difference(foundFiles.begin(), foundFiles.end(), keepFiles.begin(), keepFiles.end(), std::back_inserter(deleteFiles));

    for (auto const& deletePath : deleteFiles) {
        std::cout << "Stale output file `" << deletePath << "'\n";
        if (!dryRun) {
            string osPath = fs::path::join({_config.destinationFolderPath.c_str(), deletePath.c_str()});
            auto rs = _fileSystem.remove(osPath.c_str());
            if (rs != fs::Result::Success) {
                std::cerr << "Failed to remove `" << osPath << "'\n";
            }
        }
    }

    return true;
}

bool gm::recon::ConverterApp::isUpToDate(AssetImportRecord const& record, gm::uint64 contentHash, Converter const& converter) const noexcept {
    return record.contentHash == contentHash &&
           string_view(record.importerName) == converter.name() &&
           record.importerRevision == converter.revision();
}

bool gm::recon::ConverterApp::isUpToDate(span<AssetDependencyRecord const> records) {
    for (auto const& rec : records) {
        auto osPath = fs::path::join({_config.sourceFolderPath.c_str(), rec.path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        if (contentHash != rec.contentHash) {
            return false;
        }
    }
    return true;
}

auto gm::recon::ConverterApp::findConverter(string_view path) const -> Converter* {
    for (auto const& mapping : _converters) {
        if (mapping.predicate(path)) {
            return mapping.conveter.get();
        }
    }

    return nullptr;
}

auto gm::recon::ConverterApp::collectSourceFiles() -> vector<string> {
    if (!_fileSystem.directoryExists(_config.sourceFolderPath.c_str())) {
        std::cerr << "`" << _config.sourceFolderPath << "' does not exist or is not a directory\n";
        return {};
    }

    vector<string> files;

    auto cb = [&files](fs::FileInfo const& info) -> fs::EnumerateResult {
        if (info.type == fs::FileType::Regular) {
            files.push_back(info.path);
        }
        else if (info.type == fs::FileType::Directory) {
            return fs::EnumerateResult::Recurse;
        }
        return fs::EnumerateResult::Continue;
    };
    _fileSystem.enumerate(_config.sourceFolderPath.c_str(), cb, fs::EnumerateOptions::None);

    return files;
}
