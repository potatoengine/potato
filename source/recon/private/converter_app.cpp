// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

#include "converter_app.h"
#include "grimm/foundation/string_view.h"
#include "grimm/foundation/std_iostream.h"
#include "grimm/filesystem/path_util.h"
#include "grimm/filesystem/filesystem.h"
#include "grimm/filesystem/stream.h"
#include "grimm/assetdb/hash_cache.h"
#include "converters/convert_hlsl.h"
#include "converters/convert_copy.h"
#include "converters/convert_json.h"
#include "converters/convert_ignore.h"
#include "converters/convert_model.h"
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <set>
#include <algorithm>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_sinks.h>

#if defined(GM_PLATFORM_WINDOWS)
#    include <spdlog/sinks/msvc_sink.h>
#endif

gm::recon::ConverterApp::ConverterApp() : _programName("recon"), _hashes(_fileSystem) {
    auto console = std::make_shared<spdlog::sinks::stdout_sink_mt>();
#if defined(GM_PLATFORM_WINDOWS)
    auto debug = std::make_shared<spdlog::sinks::msvc_sink_mt>();
    _logger.reset(new spdlog::logger("recon", {console, debug}));
#else
    _logger.reset(new spdlog::logger("recon", console));
#endif
    _logger->set_pattern("%v");
}

gm::recon::ConverterApp::~ConverterApp() = default;

bool gm::recon::ConverterApp::run(span<char const*> args) {
    if (!parseArguments(_config, args, _fileSystem, *_logger)) {
        _logger->error("Failed to parse arguments");
        return false;
    }

    registerConverters();

    auto libraryPath = fs::path::join({string_view(_config.destinationFolderPath), "library$.json"});
    _outputs.push_back("library$.json");
    if (_fileSystem.fileExists(libraryPath.c_str())) {
        auto libraryReadStream = _fileSystem.openRead(libraryPath.c_str(), fs::FileOpenMode::Text);
        if (!libraryReadStream) {
            _logger->error("Failed to open asset library `{}'", libraryPath);
        }
        if (!_library.deserialize(libraryReadStream)) {
            _logger->error("Failed to load asset library `{}'", libraryPath);
        }
        _logger->info("Loaded asset library `{}'", libraryPath);
    }

    auto hashCachePath = fs::path::join({string_view(_config.destinationFolderPath), "hashes$.json"});
    _outputs.push_back("hashes$.json");
    if (_fileSystem.fileExists(hashCachePath.c_str())) {
        auto hashesReadStream = _fileSystem.openRead(hashCachePath.c_str(), fs::FileOpenMode::Text);
        if (!hashesReadStream) {
            _logger->error("Failed to open hash cache `{}'", hashCachePath);
        }
        if (!_hashes.deserialize(hashesReadStream)) {
            _logger->error("Failed to load hash cache `{}'", hashCachePath);
        }
        _logger->info("Loaded hash cache `{}'", hashCachePath);
    }

    auto sources = collectSourceFiles();

    if (sources.empty()) {
        _logger->error("No source files found");
        return false;
    }

    if (_config.sourceFolderPath.empty()) {
        _logger->error("Source directory must be specified.");
        return false;
    }
    if (_config.destinationFolderPath.empty()) {
        _logger->error("Destination directory must be specified.");
        return false;
    }
    if (_config.cacheFolderPath.empty()) {
        _logger->error("Cache directory must be specified.");
        return false;
    }

    _logger->info("Source: `{}'", _config.sourceFolderPath);
    _logger->info("Destination: `{}'", _config.destinationFolderPath);
    _logger->info("Cache: `{}'", _config.cacheFolderPath);

    if (!_fileSystem.directoryExists(_config.destinationFolderPath.c_str())) {
        if (_fileSystem.createDirectories(_config.destinationFolderPath.c_str()) != fs::Result::Success) {
            _logger->error("Failed to create `{}'", _config.destinationFolderPath);
            return false;
        }
    }

    if (!_fileSystem.directoryExists(_config.cacheFolderPath.c_str())) {
        if (_fileSystem.createDirectories(_config.cacheFolderPath.c_str()) != fs::Result::Success) {
            _logger->error("Failed to create `{}'", _config.cacheFolderPath);
            return false;
        }
    }

    bool success = convertFiles(sources);
    if (!success) {
        _logger->error("Conversion failed");
    }

    auto hashesWriteStream = _fileSystem.openWrite(hashCachePath.c_str(), fs::FileOpenMode::Text);
    if (!_hashes.serialize(hashesWriteStream)) {
        _logger->error("Failed to write hash cache `{}'", hashCachePath);
        return false;
    }
    hashesWriteStream.close();

    auto libraryWriteStream = _fileSystem.openWrite(libraryPath.c_str(), fs::FileOpenMode::Text);
    if (!_library.serialize(libraryWriteStream)) {
        _logger->error("Failed to write asset library `{}'", libraryPath);
        return false;
    }
    libraryWriteStream.close();

    if (success) {
        deleteUnusedFiles(_outputs, !_config.deleteStale);
    }

    return success;
}

void gm::recon::ConverterApp::registerConverters() {
#if GM_GPU_ENABLE_D3D11
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".hlsl"; },
                           new_box<HlslConverter>()});
#else
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".hlsl"; },
                           new_box<IgnoreConverter>()});
#endif
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".hlsli"; },
                           new_box<IgnoreConverter>()});
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".json"; },
                           new_box<JsonConverter>()});
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".png"; },
                           new_box<CopyConverter>()});
    _converters.push_back({[](string_view path) { return fs::path::extension(path) == ".obj"; },
                           new_box<ModelConverter>()});
}

bool gm::recon::ConverterApp::convertFiles(vector<string> const& files) {
    bool failed = false;

    for (auto const& path : files) {
        auto assetId = _library.pathToAssetId(string_view(path));
        auto record = _library.findRecord(assetId);

        auto osPath = fs::path::join({_config.sourceFolderPath.c_str(), path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

        Converter* converter = findConverter(string_view(path));
        if (converter == nullptr) {
            failed = true;
            _logger->error("Converter not found for `{}'", path);
            continue;
        }

        bool upToDate = record != nullptr && isUpToDate(*record, contentHash, *converter) && isUpToDate(record->sourceDependencies);
        if (upToDate) {
            _logger->info("Asset `{}' is up-to-date", path);
            for (auto const& rec : record->outputs) {
                _outputs.push_back(rec.path);
            }
            continue;
        }

        auto name = converter->name();
        _logger->info("Asset `{}' requires import ({} {})", path.c_str(), std::string_view(name.data(), name.size()), converter->revision());

        Context context(path.c_str(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str(), *_logger);
        if (!converter->convert(context)) {
            failed = true;
            _logger->error("Failed conversion for `{}'", path);
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
        _logger->info("Stale output file `{}'", deletePath);
        if (!dryRun) {
            string osPath = fs::path::join({_config.destinationFolderPath.c_str(), deletePath.c_str()});
            auto rs = _fileSystem.remove(osPath.c_str());
            if (rs != fs::Result::Success) {
                _logger->error("Failed to remove `{}'", osPath);
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
        _logger->error("`{}' does not exist or is not a directory", _config.sourceFolderPath);
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
