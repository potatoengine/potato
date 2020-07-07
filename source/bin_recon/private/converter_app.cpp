// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "converter_app.h"

#include "potato/tools/hash_cache.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <set>

up::recon::ConverterApp::ConverterApp() : _programName("recon"), _fileSystem(new_box<fs>()), _hashes(*_fileSystem), _logger("recon") {}

up::recon::ConverterApp::~ConverterApp() = default;

bool up::recon::ConverterApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (_fileSystem->fileExists(configFile)) {
        _logger.info("Loading config file `{}'", configFile);
        parseConfigFile(_config, *_fileSystem, configFile, _logger);
    }

    if (!parseArguments(_config, args, *_fileSystem, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    registerConverters();

    auto libraryPath = path::join({string_view(_config.destinationFolderPath), "library$.json"});
    _outputs.push_back("library$.json");
    if (_fileSystem->fileExists(libraryPath.c_str())) {
        auto libraryReadStream = _fileSystem->openRead(libraryPath.c_str(), FileOpenMode::Text);
        if (!libraryReadStream) {
            _logger.error("Failed to open asset library `{}'", libraryPath);
        }
        if (!_library.deserialize(libraryReadStream)) {
            _logger.error("Failed to load asset library `{}'", libraryPath);
        }
        _logger.info("Loaded asset library `{}'", libraryPath);
    }

    auto hashCachePath = path::join({string_view(_config.destinationFolderPath), "hashes$.json"});
    _outputs.push_back("hashes$.json");
    if (_fileSystem->fileExists(hashCachePath.c_str())) {
        auto hashesReadStream = _fileSystem->openRead(hashCachePath.c_str(), FileOpenMode::Text);
        if (!hashesReadStream) {
            _logger.error("Failed to open hash cache `{}'", hashCachePath);
        }
        if (!_hashes.deserialize(hashesReadStream)) {
            _logger.error("Failed to load hash cache `{}'", hashCachePath);
        }
        _logger.info("Loaded hash cache `{}'", hashCachePath);
    }

    // collect all files in the source directory for conversion
    auto sources = collectSourceFiles();

    if (sources.empty()) {
        _logger.error("No source files found");
        return false;
    }

    if (_config.sourceFolderPath.empty()) {
        _logger.error("Source directory must be specified.");
        return false;
    }
    if (_config.destinationFolderPath.empty()) {
        _logger.error("Destination directory must be specified.");
        return false;
    }
    if (_config.cacheFolderPath.empty()) {
        _logger.error("Cache directory must be specified.");
        return false;
    }

    _logger.info("Source: `{}'", _config.sourceFolderPath);
    _logger.info("Destination: `{}'", _config.destinationFolderPath);
    _logger.info("Cache: `{}'", _config.cacheFolderPath);

    if (!_fileSystem->directoryExists(_config.destinationFolderPath.c_str())) {
        if (_fileSystem->createDirectories(_config.destinationFolderPath.c_str()) != IOResult::Success) {
            _logger.error("Failed to create `{}'", _config.destinationFolderPath);
            return false;
        }
    }

    if (!_fileSystem->directoryExists(_config.cacheFolderPath.c_str())) {
        if (_fileSystem->createDirectories(_config.cacheFolderPath.c_str()) != IOResult::Success) {
            _logger.error("Failed to create `{}'", _config.cacheFolderPath);
            return false;
        }
    }

    bool success = convertFiles(sources);
    if (!success) {
        _logger.error("Conversion failed");
    }

    auto hashesWriteStream = _fileSystem->openWrite(hashCachePath.c_str(), FileOpenMode::Text);
    if (!_hashes.serialize(hashesWriteStream)) {
        _logger.error("Failed to write hash cache `{}'", hashCachePath);
        return false;
    }
    hashesWriteStream.close();

    auto libraryWriteStream = _fileSystem->openWrite(libraryPath.c_str(), FileOpenMode::Text);
    if (!_library.serialize(libraryWriteStream)) {
        _logger.error("Failed to write asset library `{}'", libraryPath);
        return false;
    }
    libraryWriteStream.close();

    if (success) {
        deleteUnusedFiles(_outputs, !_config.deleteStale);
    }

    return success;
}

void up::recon::ConverterApp::registerConverters() {
    _converterFactory.registerDefaultConverters();

#if defined(UP_GPU_ENABLE_D3D11)
    _converters.push_back({[](string_view path) { return path::extension(path) == ".hlsl"; }, _converterFactory.findConverterByName("hlsl")});
#else
    _converters.push_back({[](string_view path) { return path::extension(path) == ".hlsl"; }, _converterFactory.findConverterByName("ignore")});
#endif
    _converters.push_back({[](string_view path) { return path::extension(path) == ".hlsli"; }, _converterFactory.findConverterByName("ignore")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".txt"; }, _converterFactory.findConverterByName("ignore")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".json"; }, _converterFactory.findConverterByName("json")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".png"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".jpg"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".ttf"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".wav"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".mp3"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".popr"; }, _converterFactory.findConverterByName("ignore")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".scene"; }, _converterFactory.findConverterByName("copy")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".obj"; }, _converterFactory.findConverterByName("model")});
    _converters.push_back({[](string_view path) { return path::extension(path) == ".mat"; }, _converterFactory.findConverterByName("material")});
}

bool up::recon::ConverterApp::convertFiles(vector<string> const& files) {
    bool failed = false;

    for (auto const& path : files) {
        auto assetId = _library.pathToAssetId(string_view(path));
        auto record = _library.findRecord(assetId);

        auto osPath = path::join({_config.sourceFolderPath.c_str(), path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

        Converter* converter = findConverter(string_view(path));
        if (converter == nullptr) {
            failed = true;
            _logger.error("Converter not found for `{}'", path);
            continue;
        }

        bool upToDate = record != nullptr && isUpToDate(*record, contentHash, *converter) && isUpToDate(record->sourceDependencies);
        if (upToDate) {
            _logger.info("Asset `{}' is up-to-date", path);
            for (auto const& rec : record->outputs) {
                _outputs.push_back(rec.path);
            }
            continue;
        }

        auto name = converter->name();
        _logger.info("Asset `{}' requires import ({} {})", path.c_str(), string_view(name.data(), name.size()), converter->revision());

        ConverterContext context(path.c_str(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str(), *_fileSystem, _logger);
        checkMetafile(context, path);

        if (!converter->convert(context)) {
            failed = true;
            _logger.error("Failed conversion for `{}'", path);
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
            auto osPath = path::join({_config.sourceFolderPath.c_str(), sourceDepPath.c_str()});
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            newRecord.sourceDependencies.push_back(AssetDependencyRecord{string(sourceDepPath), contentHash});
        }

        for (auto const& outputPath : context.outputs()) {
            auto osPath = path::join({_config.destinationFolderPath.c_str(), outputPath.c_str()});
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            newRecord.outputs.push_back(AssetOutputRecord{string(outputPath), contentHash});
        }

        _library.insertRecord(std::move(newRecord));
    }

    return !failed;
}

bool up::recon::ConverterApp::deleteUnusedFiles(vector<string> const& files, bool dryRun) {
    std::set<string> keepFiles(files.begin(), files.end());
    std::set<string> foundFiles;
    auto cb = [&foundFiles](EnumerateItem const& item) {
        if (item.info.type == FileType::Regular) {
            foundFiles.insert(string(item.info.path));
        }
        return EnumerateResult::Recurse;
    };
    _fileSystem->enumerate(_config.destinationFolderPath.c_str(), cb);

    vector<string> deleteFiles;
    std::set_difference(foundFiles.begin(), foundFiles.end(), keepFiles.begin(), keepFiles.end(), std::back_inserter(deleteFiles));

    for (auto const& deletePath : deleteFiles) {
        _logger.info("Stale output file `{}'", deletePath);
        if (!dryRun) {
            string osPath = path::join({_config.destinationFolderPath.c_str(), deletePath.c_str()});
            auto rs = _fileSystem->remove(osPath.c_str());
            if (rs != IOResult::Success) {
                _logger.error("Failed to remove `{}'", osPath);
            }
        }
    }

    return true;
}

bool up::recon::ConverterApp::isUpToDate(AssetImportRecord const& record, up::uint64 contentHash, Converter const& converter) const noexcept {
    return record.contentHash == contentHash && string_view(record.importerName) == converter.name() &&
        record.importerRevision == converter.revision();
}

bool up::recon::ConverterApp::isUpToDate(span<AssetDependencyRecord const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join({_config.sourceFolderPath.c_str(), rec.path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        if (contentHash != rec.contentHash) {
            return false;
        }
    }
    return true;
}

auto up::recon::ConverterApp::findConverter(string_view path) const -> Converter* {
    for (auto const& mapping : _converters) {
        if (mapping.predicate(path)) {
            return mapping.conveter;
        }
    }

    return nullptr;
}

auto up::recon::ConverterApp::checkMetafile(ConverterContext& ctx, string_view filename) -> void {
    // check to see if a meta file exists for this asset -- if it doesn't create one
    fixed_string_writer<256> metaFile;

    metaFile.append(filename);
    metaFile.append(".meta");
    if (!_fileSystem->fileExists(metaFile.c_str())) {
        Converter* conveter = findConverter(filename);
        if (conveter != nullptr) {
            nlohmann::json root;
            string id = UUID::generate().toString();
            root["id"] = id.c_str();

            ConverterContext context(filename.data(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str(), *_fileSystem, _logger);
            string settings = conveter->generateSettings(context);
            root["settings"] = settings;

            auto stream = _fileSystem->openWrite(metaFile.c_str(), FileOpenMode::Text);
            auto json = root.dump(2);

            if (writeAllText(stream, {json.data(), json.size()}) != IOResult::Success) {
                _logger.error("Failed to write meta file for {}", metaFile.c_str());
            }
        }
    }
    // adding meta files to source deps to ensure proper rebuild when meta files change for any reason
    // (like convert settings for a file)
    ctx.addSourceDependency(metaFile.c_str());
}

auto up::recon::ConverterApp::collectSourceFiles() -> vector<string> {
    if (!_fileSystem->directoryExists(_config.sourceFolderPath.c_str())) {
        _logger.error("`{}' does not exist or is not a directory", _config.sourceFolderPath);
        return {};
    }

    vector<string> files;
    auto cb = [&files](EnumerateItem const& item) -> EnumerateResult {
        if (item.info.type == FileType::Regular) {
            // skip .meta files for now
            if (path::extension(item.info.path) == ".meta") {
                return EnumerateResult::Continue;
            }

            files.push_back(item.info.path);
        }
        else if (item.info.type == FileType::Directory) {
            return EnumerateResult::Recurse;
        }

        return EnumerateResult::Continue;
    };

    _fileSystem->enumerate(_config.sourceFolderPath.c_str(), cb, EnumerateOptions::None);
    return files;
};
