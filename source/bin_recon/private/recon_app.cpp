// Copyright by Potato Engine contributors. See accompanying License.txt for copyright details.

#include "recon_app.h"

#include "potato/format/format.h"
#include "potato/recon/file_hash_cache.h"
#include "potato/recon/recon_protocol.h"
#include "potato/tools/meta_file.h"
#include "potato/runtime/filesystem.h"
#include "potato/runtime/json.h"
#include "potato/runtime/path.h"
#include "potato/runtime/stream.h"
#include "potato/runtime/uuid.h"
#include "potato/spud/std_iostream.h"
#include "potato/spud/string_view.h"
#include "potato/spud/string_writer.h"

#include <nlohmann/json.hpp>

up::recon::ReconApp::ReconApp() : _programName("recon"), _logger("recon") {}

up::recon::ReconApp::~ReconApp() = default;

bool up::recon::ReconApp::run(span<char const*> args) {
    zstring_view const configFile = "recon.config.json";
    if (fs::fileExists(configFile)) {
        _logger.info("Loading config file `{}'", configFile);
        parseConfigFile(_config, configFile, _logger);
    }

    if (!parseArguments(_config, args, _logger)) {
        _logger.error("Failed to parse arguments");
        return false;
    }

    if (_config.server) {
        Logger::root().attach(new_shared<ReconProtocolLogSink>());
    }

    _project = Project::loadFromFile(_config.project);
    if (_project == nullptr) {
        _logger.error("Failed to load project file `{}`", _config.project);
        return false;
    }

    if (auto const rs = fs::createDirectories(_project->libraryPath()); rs != IOResult::Success) {
        _logger.error("Failed to create library folder `{}`: {}", _project->libraryPath(), rs);
        return false;
    }

    _temporaryOutputPath = path::join(_project->libraryPath(), "temp");

    _registerImporters();

    auto libraryPath = path::join(_project->libraryPath(), "assets.db");
    if (!_library.open(libraryPath)) {
        _logger.error("Failed to open asset library `{}'", libraryPath);
    }
    _logger.info("Opened asset library `{}'", libraryPath);

    auto hashCachePath = path::join(_project->libraryPath(), "hash_cache.db");
    if (!_hashes.open(hashCachePath)) {
        _logger.error("Failed to open hash cache `{}'", hashCachePath);
    }
    _logger.info("Opened hash cache `{}'", hashCachePath);

    bool success = _config.server ? _runServer() : _runOnce();

    if (!_hashes.close()) {
        _logger.error("Failed to close hash cache `{}'", hashCachePath);
        return false;
    }

    if (!_library.close()) {
        _logger.error("Failed to close library `{}'", libraryPath);
        return false;
    }

    return success;
}

bool up::recon::ReconApp::_runOnce() {
    // collect all files in the source directory for conversion
    auto sources = _collectSourceFiles();

    if (sources.empty()) {
        _logger.error("No source files found");
        return false;
    }

    bool success = _importFiles(sources);
    if (!success) {
        _logger.error("Import failed");
    }

    if (!_writeManifest()) {
        _logger.error("Failed to write manifest");
        return false;
    }

    return true;
}

bool up::recon::ReconApp::_runServer() {
    // import all the initial files
    _runOnce();

    auto [rs, watchHandle] = fs::watchDirectory(_project->resourceRootPath());
    if (rs != IOResult::Success) {
        return false;
    }

    fs::WatchHandle& handle = *watchHandle;
    std::thread waitParent([&handle] {
        char c{};
        while (std::cin.get(c) && !std::cin.eof()) {
            // consume input until there's none left (EOF/closed)
        }
        handle.close();
    });

    fs::Watch watch;
    while (handle.isOpen()) {
        watchHandle->watch(watch);

        if (watch.path.starts_with(".library")) {
            continue;
        }

        if (watch.path.ends_with(".meta")) {
            watch.path = path::changeExtension(watch.path, "");
        }

        _logger.info("Change: {}", watch.path);
        _importFile(watch.path);

        _writeManifest();
    }

    waitParent.join();

    // server only "fails" if it can't run at all
    return true;
}

void up::recon::ReconApp::_registerImporters() {
    _importerFactory.registerDefaultImporters();

    for (auto const& mapping : _config.mapping) {
        auto const importer = _importerFactory.findImporterByName(mapping.importer);
        if (importer == nullptr) {
            _logger.error("Unknown importer `{}'", mapping.importer);
        }

        string_view pattern = mapping.pattern;
        _importers.push_back(
            {[pattern](string_view filename) { return path::extension(filename) == pattern; }, importer});
    }
}

bool up::recon::ReconApp::_importFiles(view<string> files) {
    bool failed = false;

    for (auto const& path : files) {
        if (!_importFile(path)) {
            failed = true;
        }
    }

    return !failed;
}

bool up::recon::ReconApp::_importFile(zstring_view file) {
    auto assetId = _library.pathToAssetId(file);
    auto record = _library.findRecord(assetId);

    auto osPath = path::join(_project->resourceRootPath(), file.c_str());
    auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());

    Importer* importer = _findConverter(file);
    if (importer == nullptr) {
        _logger.error("Importer not found for `{}'", file);
        return false;
    }

    bool const upToDate =
        record != nullptr && _isUpToDate(*record, contentHash, *importer) && _isUpToDate(record->dependencies);
    if (upToDate) {
        _logger.info("Asset `{}' is up-to-date", file);
        return true;
    }

    bool const deleted = !fs::fileExists(osPath);
    if (deleted) {
        _logger.info("Asset `{}` is deleted", file);
        return true;
    }

    auto name = importer->name();
    _logger
        .info("Asset `{}' requires import ({} {})", file, string_view(name.data(), name.size()), importer->revision());

    ImporterContext context(file, _project->resourceRootPath(), _temporaryOutputPath, _logger);
    if (!_checkMetafile(context, file)) {
        return true;
    }

    if (!importer->import(context)) {
        _logger.error("Failed import for `{}'", file);
        return false;
    }

    AssetLibrary::Imported newRecord;
    newRecord.assetId = assetId;
    newRecord.sourcePath = string(file);
    newRecord.sourceContentHash = contentHash;
    newRecord.importerName = string(importer->name());
    newRecord.importerRevision = importer->revision();

    string_writer logicalAssetName;

    // move outputs to CAS
    //
    for (auto const& output : context.outputs()) {
        auto outputOsPath = path::join(_temporaryOutputPath, output.path);
        auto const outputHash = _hashes.hashAssetAtPath(outputOsPath);

        logicalAssetName.clear();
        format_append(
            logicalAssetName,
            "{}{}{}",
            newRecord.sourcePath,
            output.logicalAsset.empty() ? "" : ":",
            output.logicalAsset);
        auto const logicalAssetId = _library.pathToAssetId(logicalAssetName);

        newRecord.outputs.push_back({output.logicalAsset, logicalAssetId, outputHash});

        fixed_string_writer<32> casPath;
        format_append(
            casPath,
            "{:02X}/{:04X}/{:016X}.bin",
            (outputHash >> 56) & 0xFF,
            (outputHash >> 40) & 0XFFFF,
            outputHash);

        auto casOsPath = path::join(_project->libraryPath(), "cache", casPath);
        auto casOsFolder = string{path::parent(casOsPath)};

        if (auto const rs = fs::createDirectories(casOsFolder); rs != IOResult::Success) {
            _logger.error("Failed to create directory `{}`", casOsFolder);
            continue;
        }

        if (auto const rs = fs::moveFileTo(outputOsPath, casOsPath); rs != IOResult::Success) {
            _logger.error("Failed to move temp file `{}` to CAAS `{}`", outputOsPath, casOsPath);
            continue;
        }
    }

    for (auto const& sourceDepPath : context.sourceDependencies()) {
        auto osPath = path::join(_project->resourceRootPath(), sourceDepPath.c_str());
        auto const contentHash = _hashes.hashAssetAtPath(osPath.c_str());
        newRecord.dependencies.push_back({string(sourceDepPath), contentHash});
    }

    _library.insertRecord(std::move(newRecord));
    return true;
}

bool up::recon::ReconApp::_isUpToDate(
    AssetLibrary::Imported const& record,
    up::uint64 contentHash,
    Importer const& importer) const noexcept {
    return record.sourceContentHash == contentHash && string_view(record.importerName) == importer.name() &&
        record.importerRevision == importer.revision();
}

bool up::recon::ReconApp::_isUpToDate(span<AssetLibrary::Dependency const> records) {
    for (auto const& rec : records) {
        auto osPath = path::join(_project->resourceRootPath(), rec.path.c_str());
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

    string metaFileOsPath = path::join(_project->resourceRootPath(), metaFilePath);

    MetaFile metaFile;
    bool dirty = false;

    if (Stream stream = fs::openRead(metaFileOsPath, fs::OpenMode::Text); stream) {
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
            metaFile.importerName = string{importer->name()};
            dirty = true;
        }

        ImporterContext context(filename, _project->resourceRootPath(), _temporaryOutputPath, _logger);
        string_view settings = importer->generateSettings(context);
        if (settings != metaFile.importerSettings) {
            metaFile.importerSettings = string{settings};
            dirty = true;
        }
    }

    if (dirty) {
        _logger.info("Writing meta file `{}'", metaFilePath);

        string jsonText = metaFile.toJson();

        auto stream = fs::openWrite(metaFileOsPath, fs::OpenMode::Text);
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
    if (!fs::directoryExists(_project->resourceRootPath())) {
        _logger.error("`{}' does not exist or is not a directory", _project->resourceRootPath());
        return {};
    }

    vector<string> files;
    auto cb = [&files](auto const& item, int) {
        // do not recurse into the library folder
        //
        if (item.path.starts_with(".library")) {
            return fs::next;
        }

        if (item.type == fs::FileType::Regular) {
            // skip .meta files
            //
            if (path::extension(item.path) == ".meta") {
                return fs::next;
            }

            files.push_back(item.path);
        }

        return fs::recurse;
    };

    (void)fs::enumerate(_project->resourceRootPath(), cb);
    return files;
};

bool up::recon::ReconApp::_writeManifest() {
    auto manifestPath = path::join(_project->libraryPath(), "manifest.txt");
    auto manifestFile = fs::openWrite(manifestPath.c_str(), fs::OpenMode::Text);
    if (!manifestFile) {
        _logger.error("Failed to open manifest `{}'", manifestPath);
        return false;
    };
    _library.generateManifest(manifestFile);
    manifestFile.close();

    return true;
}
