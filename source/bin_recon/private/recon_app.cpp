// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"

#include "potato/tools/file_hash_cache.h"
#include "potato/tools/meta_file.h"
#include "potato/tools/resource_manifest.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/native.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>
#include <algorithm>
#include <set>

up::recon::ReconApp::ReconApp() : _programName("recon"), _fileSystem(new_box<NativeFileSystem>()), _hashes(*_fileSystem), _logger("recon") {}

up::recon::ReconApp::~ReconApp() = default;

bool up::recon::ReconApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (_fileSystem->fileExists(configFile)) {
        _logger.info("Loading config file `{}'", configFile);
        parseConfigFile(_config, *_fileSystem, configFile, _logger);
    }

    if (!parseArguments(_config, args, *_fileSystem, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    _registerImporters();

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
    auto sources = _collectSourceFiles();

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

    bool success = _importFiles(sources);
    if (!success) {
        _logger.error("Import failed");
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

    auto const manifest = _generateManifest();
    auto manifestPath = path::join(_config.destinationFolderPath, "manifest$.pom");
    _outputs.push_back("manifest$.pom");
    _logger.info("Writing manifest `{}`", manifestPath);
    auto manifestFile = _fileSystem->openWrite(manifestPath.c_str(), FileOpenMode::Text);
    if (!manifestFile || !manifest.writeManifest(manifestFile)) {
        _logger.error("Failed to write manifest `{}'", manifestPath);
        return false;
    };
    manifestFile.close();

    if (success) {
        _deleteUnusedFiles(_outputs, !_config.deleteStale);
    }

    return success;
}

void up::recon::ReconApp::_registerImporters() {
    _importerFactory.registerDefaultImporters();

    for (auto const& mapping : _config.mapping) {
        auto const importer = _importerFactory.findImporterByName(mapping.importer);
        if (importer == nullptr) {
            _logger.error("Unknown importer `{}'", mapping.importer);
        }

        string_view pattern = mapping.pattern;
        _importers.push_back({[pattern, importer](string_view filename) { return path::extension(filename) == pattern; }, importer});
    }
}

bool up::recon::ReconApp::_importFiles(view<string> files) {
    bool failed = false;

    for (auto const& path : files) {
        auto assetId = _library.pathToAssetId(string_view(path));
        auto record = _library.findRecord(assetId);

        auto osPath = path::join({_config.sourceFolderPath.c_str(), path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

        Importer* importer = _findConverter(string_view(path));
        if (importer == nullptr) {
            failed = true;
            _logger.error("Importer not found for `{}'", path);
            continue;
        }

        bool upToDate = record != nullptr && _isUpToDate(*record, contentHash, *importer) && _isUpToDate(record->sourceDependencies);
        if (upToDate) {
            _logger.info("Asset `{}' is up-to-date", path);
            for (auto const& rec : record->outputs) {
                _outputs.push_back(rec.path);
            }
            continue;
        }

        auto name = importer->name();
        _logger.info("Asset `{}' requires import ({} {})", path.c_str(), string_view(name.data(), name.size()), importer->revision());

        ImporterContext context(path.c_str(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str(), *_fileSystem, _logger);
        if (!_checkMetafile(context, path)) {
            continue;
        }

        if (!importer->import(context)) {
            failed = true;
            _logger.error("Failed import for `{}'", path);
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
        newRecord.importerName = string(importer->name());
        newRecord.importerRevision = importer->revision();

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

bool up::recon::ReconApp::_deleteUnusedFiles(view<string> files, bool dryRun) {
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

bool up::recon::ReconApp::_isUpToDate(AssetImportRecord const& record, up::uint64 contentHash, Importer const& importer) const noexcept {
    return record.contentHash == contentHash && string_view(record.importerName) == importer.name() && record.importerRevision == importer.revision();
}

bool up::recon::ReconApp::_isUpToDate(span<AssetDependencyRecord const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join({_config.sourceFolderPath.c_str(), rec.path.c_str()});
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        if (contentHash != rec.contentHash) {
            return false;
        }
    }
    return true;
}

auto up::recon::ReconApp::_findConverter(string_view path) const -> Importer* {
    for (auto const& mapping : _importers) {
        if (mapping.predicate(path)) {
            return mapping.conveter;
        }
    }

    return nullptr;
}

auto up::recon::ReconApp::_checkMetafile(ImporterContext& ctx, string_view filename) -> bool {
    string_writer metaFilePath;
    metaFilePath.append(filename);
    metaFilePath.append(".meta");

    string metaFileOsPath = path::join(_config.sourceFolderPath, metaFilePath);

    MetaFile metaFile;
    bool dirty = false;

    if (Stream stream = _fileSystem->openRead(metaFileOsPath, FileOpenMode::Text); stream) {
        auto [result, jsonText] = readText(stream);
        if (result == IOResult::Success) {
            if (!metaFile.parseJson(jsonText)) {
                metaFile.generate();
                dirty = true;
            }
        }
        else {
            metaFile.generate();
            dirty = true;
        }
    }
    else {
        metaFile.generate();
        dirty = true;
    }

    Importer* importer = _findConverter(filename);
    if (importer != nullptr) {
        if (importer->name() != string_view{metaFile.importerName}) {
            metaFile.importerName = importer->name();
            dirty = true;
        }

        ImporterContext context(filename.data(), _config.sourceFolderPath.c_str(), _config.destinationFolderPath.c_str(), *_fileSystem, _logger);
        string settings = importer->generateSettings(context);
        if (settings != metaFile.importerSettings) {
            metaFile.importerSettings = std::move(settings);
            dirty = true;
        }
    }

    if (dirty) {
        _logger.info("Writing meta file `{}'", metaFilePath);

        string jsonText = metaFile.toJson();

        auto stream = _fileSystem->openWrite(metaFileOsPath, FileOpenMode::Text);
        if (!stream || writeAllText(stream, jsonText) != IOResult::Success) {
            _logger.error("Failed to write meta file for {}", metaFilePath);
            return false;
        }
    }

    // adding meta files to source deps to ensure proper rebuild when meta files change for any reason
    // (like convert settings for a file)
    ctx.addSourceDependency(metaFilePath);
    return true;
}

auto up::recon::ReconApp::_collectSourceFiles() -> vector<string> {
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

auto up::recon::ReconApp::_generateManifest() -> ResourceManifest { return _library.generateManifest(); }
