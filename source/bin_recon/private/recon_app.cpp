// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"

#include "potato/format/format.h"
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

    _libraryPath = path::join(_config.sourceFolderPath, ".library");
    if (auto const rs = _fileSystem->createDirectories(_libraryPath); rs != IOResult::Success) {
        _logger.error("Failed to create library folder `{}`: {}", _libraryPath, rs);
        return false;
    }

    _temporaryOutputPath = path::join(_libraryPath, "temp");

    _registerImporters();

    auto libraryPath = path::join(_libraryPath, "assets.json");
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

    auto hashCachePath = path::join(_libraryPath, "hashes.json");
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

    _logger.info("Source at `{}'", _config.sourceFolderPath);

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
    auto manifestPath = path::join(_libraryPath, "manifest.txt");
    auto manifestFile = _fileSystem->openWrite(manifestPath.c_str(), FileOpenMode::Text);
    if (!manifestFile || !manifest.writeManifest(manifestFile)) {
        _logger.error("Failed to write manifest `{}'", manifestPath);
        return false;
    };
    manifestFile.close();

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
            continue;
        }

        auto name = importer->name();
        _logger.info("Asset `{}' requires import ({} {})", path.c_str(), string_view(name.data(), name.size()), importer->revision());

        ImporterContext context(path, _config.sourceFolderPath, _temporaryOutputPath, *_fileSystem, _logger);
        if (!_checkMetafile(context, path)) {
            continue;
        }

        if (!importer->import(context)) {
            failed = true;
            _logger.error("Failed import for `{}'", path);
            continue;
        }

        AssetImportRecord newRecord;
        newRecord.assetId = assetId;
        newRecord.path = string(path);
        newRecord.contentHash = contentHash;
        newRecord.category = AssetCategory::Source;
        newRecord.importerName = string(importer->name());
        newRecord.importerRevision = importer->revision();

        // move outputs to CAS
        //
        for (auto const& output : context.outputs()) {
            auto outputOsPath = path::join(_temporaryOutputPath, output.path);
            auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

            fixed_string_writer<64> casPath;
            format_append(casPath,
                "{:02X}/{:04X}/{:010X}{}",
                (outputHash >> 56) & 0xFF,
                (outputHash >> 40) & 0XFFFF,
                outputHash & 0xFF'FFFF'FFFF,
                path::extension(string_view{output.path}));

            auto casOsPath = path::join(_libraryPath, "cache", casPath);
            string casOsFolder = path::parent(casOsPath);

            _fileSystem->createDirectories(casOsFolder);
            _fileSystem->moveFileTo(outputOsPath, casOsPath);

            string_writer logicalAssetName;
            format_append(logicalAssetName, "{}:{}", newRecord.path, output.logicalAsset);
            auto const logicalAssetId = _library.pathToAssetId(logicalAssetName);

            newRecord.outputs.push_back(AssetOutputRecord{logicalAssetId, outputHash});
        }

        for (auto const& sourceDepPath : context.sourceDependencies()) {
            auto osPath = path::join({_config.sourceFolderPath.c_str(), sourceDepPath.c_str()});
            auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
            newRecord.sourceDependencies.push_back(AssetDependencyRecord{string(sourceDepPath), contentHash});
        }

        _library.insertRecord(std::move(newRecord));
    }

    return !failed;
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

auto up::recon::ReconApp::_checkMetafile(ImporterContext& ctx, zstring_view filename) -> bool {
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

        ImporterContext context(filename, _config.sourceFolderPath, _temporaryOutputPath, *_fileSystem, _logger);
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
        // do not recurse into the library folder
        //
        if (item.info.path.starts_with(".library")) {
            return EnumerateResult::Continue;
        }

        if (item.info.type == FileType::Regular) {
            // skip .meta files
            //
            if (path::extension(item.info.path) == ".meta") {
                return EnumerateResult::Continue;
            }

            files.push_back(item.info.path);
        }

        return EnumerateResult::Recurse;
    };

    _fileSystem->enumerate(_config.sourceFolderPath.c_str(), cb, EnumerateOptions::None);
    return files;
};

auto up::recon::ReconApp::_generateManifest() -> ResourceManifest { return _library.generateManifest(); }
